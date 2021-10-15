// Copyright 2013 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ========================================================================

#include "omaha/enterprise/installer/custom_actions/msi_tag_extractor.h"

#include <windows.h>
#include <algorithm>

namespace {

// A fixed string serves as the magic number to identify whether it's the start
// of a valid tag.
const char kTagMagicNumber[] = "Gact";

const size_t kMagicNumberLength = arraysize(kTagMagicNumber) - 1;
const char kStringMapDelimeter = '&';
const char kKeyValueDelimeter = '=';

const DWORD kMaxTagStringLengthAllowed = 4096;

// Tag should at least have magic number "Gact" plus tag string length (uint16)
// and the length of the last integer(uint32).
const DWORD kMinTagLengthAllowed = static_cast<DWORD>(
    kMagicNumberLength + sizeof(uint16) + sizeof(uint32));  // NOLINT

const DWORD kMaxTagLength = kMaxTagStringLengthAllowed + kMinTagLengthAllowed;

// Read a value from the given buffer. The layout is assumed to be
// big-endian. Caller must make sure the buffer has enough length.
template<typename T>
T GetValueFromBuffer(const void* buffer) {
  const uint8* value_buffer = static_cast<const uint8*>(buffer);
  T value = T();
  for (size_t i = 0; i < sizeof(T); ++i) {
    value <<= 8;
    value |= value_buffer[i];
  }

  return value;
}

bool is_not_alnum(char c) {
  return !isalnum(c);
}

bool IsValidTagKey(const std::string& str) {
  return find_if(str.begin(), str.end(), is_not_alnum) == str.end();
}

bool is_not_valid_value_char(char c) {
  if (isalnum(c)) {
    return false;
  }
  const char kValidChars[] = "{}[]-% ";
  return strchr(kValidChars, c) == NULL;
}

bool IsValidTagValue(const std::string& str) {
  return find_if(str.begin(), str.end(), is_not_valid_value_char) == str.end();
}

}  // namespace

namespace custom_action {

MsiTagExtractor::MsiTagExtractor() {
}

bool MsiTagExtractor::ReadTagFromFile(const wchar_t* filename) {
  HANDLE file_handle = NULL;

  file_handle = ::CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file_handle == INVALID_HANDLE_VALUE) {
    return false;
  }

  std::vector<char> tag_buffer;
  bool result = ReadTagToBuffer(file_handle, &tag_buffer) &&
                ParseTagBuffer(tag_buffer);

  ::CloseHandle(file_handle);
  return result;
}

// Load last 4K bytes from file, search for magic number 'Gact' in the
// buffer. If found, assume that's the start of tag and continue parsing.
bool MsiTagExtractor::ReadTagToBuffer(HANDLE file_handle,
                                      std::vector<char>* tag) const {
  // Assume file doesn't have tag if it is too small.
  LARGE_INTEGER file_size = {0};
  if (!::GetFileSizeEx(file_handle, &file_size) ||
      file_size.LowPart < kMaxTagLength) {
    return false;
  }

  const LONG kSeekOffset = -static_cast<LONG>(kMaxTagLength);
  DWORD low_ptr = ::SetFilePointer(file_handle, kSeekOffset, NULL, FILE_END);
  if (low_ptr == INVALID_SET_FILE_POINTER || ::GetLastError() != NO_ERROR) {
    return false;
  }

  // Read the last 4K bytes from file.
  std::vector<char> buffer;
  buffer.resize(kMaxTagLength + 1);
  DWORD num_bytes_read = 0;
  if (!::ReadFile(file_handle,
                  &buffer[0],
                  kMaxTagLength,
                  &num_bytes_read,
                  NULL) ||
      num_bytes_read != kMaxTagLength) {
    return false;
  }

  // Search magic number in the loaded buffer.
  char* magic_number_pos = std::search(&buffer[0],
                                       &buffer[kMaxTagLength],
                                       kTagMagicNumber,
                                       kTagMagicNumber + kMagicNumberLength);
  // Make sure we found the magic number, and the buffer after it is no less
  // than the required minimum tag size (kMinTagLengthAllowed).
  if (magic_number_pos > &buffer[0] + kMaxTagLength - kMinTagLengthAllowed) {
    return false;
  }

  tag->assign(magic_number_pos, &buffer[kMaxTagLength]);
  return true;
}

bool MsiTagExtractor::ParseTagBuffer(const std::vector<char>& tag_buffer) {
  size_t parse_position(0);
  if (!ParseMagicNumber(tag_buffer, &parse_position)) {
    return false;
  }

  const uint16 tag_length = ParseTagLength(tag_buffer, &parse_position);

  // Tag length should not exceed remaining buffer size, and bounded by
  // kMaxTagStringLengthAllowed.
  if (tag_length > tag_buffer.size() - parse_position ||
      tag_length > kMaxTagStringLengthAllowed) {
    return false;
  }

  ParseSimpleAsciiStringMap(tag_buffer, &parse_position, tag_length);
  return ParseTagEndInt(tag_buffer, &parse_position);
}

bool MsiTagExtractor::ParseMagicNumber(
    const std::vector<char>& tag_buffer, size_t* parse_position) const {
  if (tag_buffer.size() <= kMagicNumberLength + *parse_position) {
    return false;
  }

  bool result = memcmp(&tag_buffer[*parse_position],
                       kTagMagicNumber,
                       kMagicNumberLength) == 0;
  *parse_position += kMagicNumberLength;
  return result;
}

uint16 MsiTagExtractor::ParseTagLength(
    const std::vector<char>& tag_buffer, size_t* parse_position) const {
  uint16 tag_length = GetValueFromBuffer<uint16>(&tag_buffer[*parse_position]);
  *parse_position += sizeof(tag_length);
  return tag_length;
}

// Simple ASCII string map spec:
// 1. Must in format: "key1=value1&key2=value2&....keyN=valueN"
// 2. All key/value must be alpha-numeric strings, while value can be empty.
// Unrecognized tag will be ignored.
void MsiTagExtractor::ParseSimpleAsciiStringMap(
    const std::vector<char>& tag_buffer,
    size_t* parse_position,
    size_t tag_length) {
  // Construct a string with at most |tag_length| characters, but stop at the
  // first '\0' if it comes before that.
  std::string tag_string(std::string(&tag_buffer[*parse_position],
                                     tag_length).c_str());

  size_t start_pos = 0;
  size_t found = tag_string.find_first_of(kStringMapDelimeter, start_pos);
  while (found != std::string::npos) {
    ParseKeyValueSubstring(tag_string.substr(start_pos, found - start_pos));
    start_pos = found + 1;
    found = tag_string.find_first_of(kStringMapDelimeter, start_pos);
  }
  ParseKeyValueSubstring(tag_string.substr(start_pos));
  *parse_position += tag_length;
}

void MsiTagExtractor::ParseKeyValueSubstring(const std::string& key_value_str) {
  size_t found = key_value_str.find(kKeyValueDelimeter);
  if (found == std::string::npos) {
    return;
  }
  std::string key = key_value_str.substr(0, found);
  std::string value = key_value_str.substr(found + 1);

  if (IsValidTagKey(key) && IsValidTagValue(value) && !key.empty()) {
    tag_map_[key] = value;
  }
}

bool MsiTagExtractor::GetValue(const char* key, std::string* value) const {
  std::map<std::string, std::string>::const_iterator it = tag_map_.find(key);
  if (it == tag_map_.end()) {
    return false;
  }

  *value = it->second;
  return true;
}

bool MsiTagExtractor::ParseTagEndInt(
    const std::vector<char>& tag_buffer, size_t* parse_position) const {
  uint32 end_int = GetValueFromBuffer<uint32>(&tag_buffer[*parse_position]);
  *parse_position += sizeof(end_int);
  // The integer must be 0.
  return end_int == 0;
}

}  // namespace custom_action

