/**
Copyright 2017 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS-IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#ifndef ION_GFX_TRACECALLEXTRACTOR_H_
#define ION_GFX_TRACECALLEXTRACTOR_H_

#include <map>
#include <string>
#include <vector>

namespace ion {
namespace gfx {

/// The TraceCallExtractor class parses an OpenGL trace output generated by a
/// GraphicsManager, segmenting it into a vector of calls with arguments. It also
/// contains utility functions to locate and count particular calls. Calls are
/// tested using one or more strings that must match the beginnings of the
/// resulting trace strings. This scheme is used, rather than exact full-string
/// matches, to allow for differences in trace formatting on different platforms.
class TraceCallExtractor {
 public:
  using ArgSpec = std::vector<std::pair<int, std::string>>;
  TraceCallExtractor();
  /// Utility constructor also sets trace string and extracts vector of calls.
  explicit TraceCallExtractor(const std::string& trace);
  ~TraceCallExtractor() {}

  /// Sets trace string and extracts vector of calls.
  void SetTrace(const std::string& trace);

  const std::vector<std::string>& GetCalls() { return calls_; }

  /// Returns the number of OpenGL calls in the stream.
  size_t GetCallCount() const;

  /// Returns the number of times the passed call start occurs in the trace
  /// stream.
  size_t GetCountOf(const std::string& call_prefix) const;
  /// Returns the number of calls with the specified name and/or arguments.
  /// Key 0 in the map specifies the call name.
  size_t GetCountOf(const ArgSpec& name_and_args) const;

  /// Returns the index of the nth call in the trace stream beginning with
  /// start, if it exists, otherwise returns base::kInvalidIndex. Note that
  /// n == 0 returns the first index, n == 1 returns the second index, and so on.
  size_t GetNthIndexOf(size_t n, const std::string& call_prefix) const;
  /// Returns the index of the nth call that matches the specified name and/or
  /// arguments. Key 0 in the map specifies the name.
  size_t GetNthIndexOf(size_t n, const ArgSpec& name_and_args) const;

 private:
  /// Parses the trace stream and returns a vector of calls, removing leading
  /// whitespace from each call and ignoring markers that start with - or >.
  void CreateCallVector();

  /// OpenGL trace string
  std::string trace_;

  /// Calls extracted from current trace stream
  std::vector<std::string> calls_;
  std::vector<std::vector<std::string>> args_;
};

}  // namespace gfx
}  // namespace ion

#endif  // ION_GFX_TRACECALLEXTRACTOR_H_
