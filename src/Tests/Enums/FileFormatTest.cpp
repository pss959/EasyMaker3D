//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Enums/FileFormat.h"
#include "Tests/Testing.h"
#include "Util/Enum.h"

TEST(FileFormatTest, GetFileFormatExtension) {
    EXPECT_EQ("",     GetFileFormatExtension(FileFormat::kUnknown));
    EXPECT_EQ(".stl", GetFileFormatExtension(FileFormat::kTextSTL));
    EXPECT_EQ(".stl", GetFileFormatExtension(FileFormat::kBinarySTL));
    EXPECT_EQ(".off", GetFileFormatExtension(FileFormat::kOFF));
}
