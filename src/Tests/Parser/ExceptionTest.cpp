//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tests/Testing.h"
#include "Parser/Exception.h"
#include "Util/FilePath.h"

TEST(ExceptionTest, Constructors) {
    Parser::Exception ex1("dummy message");
    EXPECT_EQ(Str("dummy message"), Str(ex1.what()));

    Parser::Exception ex2(FilePath("/a/b/c/d"), "another message");
    EXPECT_EQ(Str("\n/a/b/c/d: Parse error: another message"), Str(ex2.what()));

    Parser::Exception ex3(FilePath("/a/b/c/d"), 123, "third message");
    EXPECT_EQ(Str("\n/a/b/c/d:123: Parse error: third message"),
              Str(ex3.what()));
}
