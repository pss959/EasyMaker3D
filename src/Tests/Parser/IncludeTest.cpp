//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tests/Parser/ParserTestBase.h"
#include "Tests/TempFile.h"
#include "Util/FilePath.h"

/// \ingroup Tests
class IncludeTest : public ParserTestBase {};

TEST_F(IncludeTest, Includes) {
    TempFile file1("Simple \"Child1\" {}");
    TempFile file2("Simple \"Child2\" {}");

    // Use the absolute path to the first file.
    const FilePath path1 = file1.GetPath();

    // Set the base path for converting relative paths to absolute and get the
    // relative path to the second file to test relative file processing.
    const FilePath base_path = path1.GetParentDirectory();
    parser.SetBasePath(base_path);
    const auto path2 = file2.GetPath().MakeRelativeTo(base_path);

    const Str input =
        "Derived \"ParentName\" { \n"
        "  simple_list: [\n"
        "      <\"" + path1.ToString() + "\">,\n"
        "      <\"" + path2.ToString() + "\">,\n"
        "  ],\n"
        "}\n";

    EXPECT_EQ(0U, parser.GetDependencies().size());

    Parser::ObjectPtr obj = ParseString(input);
    EXPECT_NOT_NULL(obj.get());

    EXPECT_EQ("Derived", obj->GetTypeName());
    EXPECT_EQ("ParentName", obj->GetName());
    DerivedPtr dp = std::dynamic_pointer_cast<Derived>(obj);
    EXPECT_NOT_NULL(dp.get());
    const std::vector<SimplePtr> &list = dp->simple_list;
    EXPECT_FALSE(list.empty());
    EXPECT_EQ(2U, list.size());
    EXPECT_EQ("Child1", list[0]->GetName());
    EXPECT_EQ("Child2", list[1]->GetName());

    // There should be 2 dependencies based on the included files.
    const auto deps = parser.GetDependencies();
    EXPECT_EQ(2U,    deps.size());
    EXPECT_EQ("",    deps[0].including_path);
    EXPECT_EQ(path1, deps[0].included_path);
    EXPECT_EQ("",    deps[1].including_path);
    EXPECT_EQ(path2, deps[1].included_path);
}
