//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "App/Args.h"
#include "Tests/Testing2.h"

constinit const char kUsageString[] =
R"(ArgsTest: Tests Args class.

    Usage:
      ArgsTest [--bashful=<n>] [--doc] [--dopey=<n>] [--grumpy=<s>] [--happy]
               [--sleepy=<s>] [--sneezy=<n>]

    Options:
      --bashful=<n> A number.
      --doc         Something.
      --dopey=<n>   Description.
      --grumpy<s>   More.
      --happy       Another.
      --sleepy=<s>  Useless.
      --sneezy=<n>  Same.

    More text here for no reason.
)";

TEST(ArgsTest, Args) {
    const char *argv[]{
        "ArgsTest",
        "--happy",
        "--dopey=12",
        "--sleepy=tired",
        "--bashful=abadnumber",
    };
    Args args(4, argv, kUsageString);

    EXPECT_TRUE(args.HasArg("--happy"));
    EXPECT_TRUE(args.HasArg("--sneezy"));
    EXPECT_FALSE(args.HasArg("--nosuch"));
    TEST_ASSERT(args.GetString("--nosuch"), "HasArg");

    EXPECT_EQ("tired", args.GetString("--sleepy"));
    EXPECT_EQ("",      args.GetString("--grumpy"));
    EXPECT_EQ("",      args.GetString("--happy"));
    EXPECT_EQ("12",    args.GetString("--dopey"));  // Anything can be a string.

    EXPECT_EQ("tired", args.GetStringChoice(
                  "--sleepy", StrVec{ "bored", "tired", "exhausted" }));
    EXPECT_EQ("bored", args.GetStringChoice(   // Not specified => 1st choice.
                  "--grumpy", StrVec{ "bored", "tired", "exhausted" }));
    TEST_THROW(args.GetStringChoice(
                   "--sleepy", StrVec{ "no", "good", "choice" }),
               docopt::DocoptArgumentError, "Invalid choice");

    EXPECT_TRUE(args.GetBool("--happy"));
    EXPECT_FALSE(args.GetBool("--doc"));           // Not specified.
    EXPECT_FALSE(args.GetBool("--grumpy"));        // Not a bool.
    TEST_ASSERT(args.GetBool("--nosuch"), "HasArg");

    EXPECT_EQ(12, args.GetAsInt("--dopey",   42));
    EXPECT_EQ(42, args.GetAsInt("--sneezy",  42));  // Not specified.
    EXPECT_EQ(42, args.GetAsInt("--bashful", 42));  // Cannot convert.
    TEST_ASSERT(args.GetAsInt("--nosuch", 19), "HasArg");
}
