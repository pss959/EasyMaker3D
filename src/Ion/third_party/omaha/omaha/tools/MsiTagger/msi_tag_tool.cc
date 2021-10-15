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
//
// This is a naive tool to write tags to MSI packages. It just blindly adds
// tags without checking:
//   1. Whether the input file is a valid MSI package.
//   2. Whether the file is tagged already.

// Tag format:
//
// +-------------------------------------+
// ~    ..............................   ~
// |    ..............................   |
// | End of MSI file                     | End of raw MSI.
// +-------------------------------------+
// | Magic number 'Gact'                 | Tag starts
// | Tag string length                   |
// | tag string                          |
// | 4 bytes of 0s.                      |
// +-------------------------------------+

#include <fstream>  // NOLINT(readability/streams)
#include "omaha/base/file.h"
#include "omaha/base/path.h"
#include "omaha/base/utils.h"

using omaha::CreateDir;
using omaha::ConcatenatePath;
using omaha::File;
using omaha::GetCurrentDir;
using omaha::GetDirectoryFromPath;
using omaha::GetFileFromPath;

// Output a uint16 to the file. Always in big endian.
void write_uint16(std::ofstream* file, uint16 value) {
  char c = static_cast<char>((value & 0xFF00) >> 8);
  file->write(&c, 1);

  c = static_cast<char>(value & 0x00FF);
  file->write(&c, 1);
}

int WriteMsiTag(
    const TCHAR* in_file,
    const TCHAR* out_file,
    const TCHAR* tag) {
  std::ifstream in;
  std::ofstream out;

  in.open(in_file, std::ifstream::in | std::ifstream::binary);
  out.open(out_file, std::ofstream::out | std::ofstream::binary);
  if (!in.is_open() || !out.is_open()) {
    return -1;
  }

  // Copy MSI package first.
  out << in.rdbuf();

  // Then append the tags to the end of file.
  const char kMagicNumber[] = "Gact";
  out.write(kMagicNumber, arraysize(kMagicNumber) - 1);

  // Write tag length.
  int tag_length = lstrlen(tag) + 1;
  write_uint16(&out, static_cast<uint16>(tag_length));

  // Actual tag.
  std::wstring tag_w(tag);
  std::string tag_ansi(tag_w.begin(), tag_w.end());
  out.write(tag_ansi.data(), tag_length);

  // Fill 4 bytes of 0s at the end.
  const char kFillBytes[4] = {0};
  out.write(kFillBytes, arraysize(kFillBytes));

  in.close();
  out.close();
  return S_OK;
}

int _tmain(int argc, _TCHAR* argv[]) {
  if (argc != 4) {
    _tprintf(_T("Incorrect number of arguments!\n"));
    _tprintf(_T("Usage: MsiTagger <source_msi> <outputfile> <tag>\n"));
    _tprintf(_T("Example: MsiTagger Setup.msi TaggedSetup.msi BRAND=GGLS\n"));
    return -1;
  }

  const TCHAR* file = argv[1];
  if (!File::Exists(file)) {
    _tprintf(_T("File \"%s\" not found!\n"), file);
    return -1;
  }

  CString dir = GetDirectoryFromPath(argv[2]);
  CString path = ConcatenatePath(GetCurrentDir(), dir);
  ASSERT1(!path.IsEmpty());
  if (!File::Exists(path)) {
    HRESULT hr = CreateDir(path, NULL);
    if (FAILED(hr)) {
      _tprintf(_T("Could not create dir %s\n"),
          static_cast<const TCHAR*>(path));
      return hr;
    }
  }
  ASSERT1(File::Exists(path));

  CString file_name = GetFileFromPath(argv[2]);
  CString out_path = ConcatenatePath(path, file_name);
  ASSERT1(!out_path.IsEmpty());

  return WriteMsiTag(file, out_path, argv[3]);
}
