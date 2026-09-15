// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef NINJA_EARLY_OUTPUT_H_
#define NINJA_EARLY_OUTPUT_H_

#include <string>
#include <vector>

/// The environment variable through which a command learns the value of its
/// edge's `early_output_prefix`. Set only for commands whose output Ninja
/// reads with that prefix in effect, so a command that finds it unset knows
/// not to announce anything.
extern const char kEarlyOutputPrefixEnvVar[];

/// Finds the early-output announcements in the output of a running command:
/// complete lines that start with the edge's `early_output_prefix`. Fed the
/// growing output buffer after every read; each byte is examined once.
struct EarlyOutputParser {
  explicit EarlyOutputParser(const std::string& prefix) : prefix_(prefix) {}

  /// False for the empty prefix: Parse() then leaves the output alone.
  bool enabled() const { return !prefix_.empty(); }

  const std::string& prefix() const { return prefix_; }

  /// Examine what was appended to |output| since the last call. Every
  /// complete line that starts with the prefix is removed from |output|, and
  /// the rest of the line (without its "\n" or "\r\n") is appended to
  /// |paths|. A last line that lacks its newline is left for the next call;
  /// if the output ends there, it stays in the output.
  void Parse(std::string* output, std::vector<std::string>* paths);

 private:
  std::string prefix_;
  /// Start of the first line of the output not yet classified.
  size_t line_start_ = 0;
  /// There is no newline in [line_start_, searched_).
  size_t searched_ = 0;
};

#endif  // NINJA_EARLY_OUTPUT_H_
