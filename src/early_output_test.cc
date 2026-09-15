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

#include "test.h"

namespace {

/// Feed |chunks| to a parser one at a time, as successive reads would.
struct Parsed {
  Parsed(const std::string& prefix, const std::vector<std::string>& chunks) {
    EarlyOutputParser parser(prefix);
    for (const std::string& chunk : chunks) {
      output += chunk;
      parser.Parse(&output, &paths);
    }
  }
  std::string output;
  std::vector<std::string> paths;
};

}  // namespace

TEST(EarlyOutputParser, DisabledLeavesOutputAlone) {
  Parsed p("", { "@ready@a.meta\n", "text\n" });
  EXPECT_EQ("@ready@a.meta\ntext\n", p.output);
  EXPECT_TRUE(p.paths.empty());
  EXPECT_FALSE(EarlyOutputParser("").enabled());
  EXPECT_TRUE(EarlyOutputParser("@ready@").enabled());
}

TEST(EarlyOutputParser, RemovesAnnouncementsKeepsTheRest) {
  Parsed p("@ready@",
           { "before\n@ready@out/a.meta\nbetween\n@ready@b\nafter\n" });
  EXPECT_EQ("before\nbetween\nafter\n", p.output);
  ASSERT_EQ(2u, p.paths.size());
  EXPECT_EQ("out/a.meta", p.paths[0]);
  EXPECT_EQ("b", p.paths[1]);
}

TEST(EarlyOutputParser, FirstAndLastLine) {
  Parsed p("@ready@", { "@ready@first\nmiddle\n@ready@last\n" });
  EXPECT_EQ("middle\n", p.output);
  ASSERT_EQ(2u, p.paths.size());
  EXPECT_EQ("first", p.paths[0]);
  EXPECT_EQ("last", p.paths[1]);
}

TEST(EarlyOutputParser, PrefixMustStartTheLine) {
  Parsed p("@ready@",
           { "note: @ready@a\n", " @ready@b\n", "@ready\n", "@read" });
  EXPECT_EQ("note: @ready@a\n @ready@b\n@ready\n@read", p.output);
  EXPECT_TRUE(p.paths.empty());
}

TEST(EarlyOutputParser, SplitAcrossReads) {
  Parsed p("@ready@", { "x\n@re", "ady@a.", "meta", "\ny\n@ready@b\n" });
  EXPECT_EQ("x\ny\n", p.output);
  ASSERT_EQ(2u, p.paths.size());
  EXPECT_EQ("a.meta", p.paths[0]);
  EXPECT_EQ("b", p.paths[1]);
}

TEST(EarlyOutputParser, CRLF) {
  Parsed p("@ready@", { "one\r\n@ready@a.meta\r\ntwo\r\n" });
  EXPECT_EQ("one\r\ntwo\r\n", p.output);
  ASSERT_EQ(1u, p.paths.size());
  EXPECT_EQ("a.meta", p.paths[0]);
}

TEST(EarlyOutputParser, PathIsTakenVerbatim) {
  Parsed p("@ready@", { "@ready@ a b$.meta \n@ready@\n" });
  EXPECT_EQ("", p.output);
  ASSERT_EQ(2u, p.paths.size());
  EXPECT_EQ(" a b$.meta ", p.paths[0]);
  EXPECT_EQ("", p.paths[1]);
}

TEST(EarlyOutputParser, UnterminatedLastLineStaysInTheOutput) {
  Parsed p("@ready@", { "text\n@ready@a.meta" });
  EXPECT_EQ("text\n@ready@a.meta", p.output);
  EXPECT_TRUE(p.paths.empty());
}

// A long line arriving in many reads is searched once, not once per read:
// this finishes instantly, where rescanning the line on every read would take
// on the order of a minute.
TEST(EarlyOutputParser, LongLineInManyReads) {
  EarlyOutputParser parser("@ready@");
  std::string output;
  std::vector<std::string> paths;
  const std::string chunk(4096, 'a');
  for (int i = 0; i < 50000; ++i) {
    output += chunk;
    parser.Parse(&output, &paths);
  }
  output += "\n@ready@a.meta\n";
  parser.Parse(&output, &paths);
  EXPECT_EQ(50000u * 4096u + 1u, output.size());
  ASSERT_EQ(1u, paths.size());
  EXPECT_EQ("a.meta", paths[0]);
}
