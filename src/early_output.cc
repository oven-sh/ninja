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

#include "early_output.h"

const char kEarlyOutputPrefixEnvVar[] = "NINJA_EARLY_OUTPUT_PREFIX";

void EarlyOutputParser::Parse(std::string* output,
                              std::vector<std::string>* paths) {
  if (prefix_.empty())
    return;
  for (;;) {
    size_t newline = output->find('\n', searched_);
    if (newline == std::string::npos) {
      searched_ = output->size();
      return;
    }
    // The prefix holds no newline, so a line shorter than it cannot match.
    if (output->compare(line_start_, prefix_.size(), prefix_) == 0) {
      size_t begin = line_start_ + prefix_.size();
      size_t end = newline;
      if (end > begin && (*output)[end - 1] == '\r')
        --end;
      paths->push_back(output->substr(begin, end - begin));
      output->erase(line_start_, newline + 1 - line_start_);
      searched_ = line_start_;
    } else {
      line_start_ = searched_ = newline + 1;
    }
  }
}
