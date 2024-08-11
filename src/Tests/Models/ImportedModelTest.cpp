//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Items/UnitConversion.h"
#include "Math/MeshValidation.h"
#include "Models/ImportedModel.h"
#include "Tests/Testing.h"
#include "Tests/SceneTestBase.h"

/// \ingroup Tests
class ImportedModelTest : public SceneTestBase {};

TEST_F(ImportedModelTest, Defaults) {
    auto imported = Model::CreateModel<ImportedModel>();
    EXPECT_FALSE(imported->CanSetComplexity());
    EXPECT_TRUE(imported->GetPath().empty());
    EXPECT_EQ(UnitConversion::Units::kCentimeters,
              imported->GetUnitConversion().GetFromUnits());
    EXPECT_EQ(UnitConversion::Units::kCentimeters,
              imported->GetUnitConversion().GetToUnits());
}

TEST_F(ImportedModelTest, UnitConversion) {
    auto imported = Model::CreateModel<ImportedModel>();
    imported->SetUnitConversion(*UnitConversion::CreateWithUnits(
                                    UnitConversion::Units::kFeet,
                                    UnitConversion::Units::kInches));
    EXPECT_EQ(UnitConversion::Units::kFeet,
              imported->GetUnitConversion().GetFromUnits());
    EXPECT_EQ(UnitConversion::Units::kInches,
              imported->GetUnitConversion().GetToUnits());
}

TEST_F(ImportedModelTest, Import) {
    auto imported = Model::CreateModel<ImportedModel>();

    // Note that calling GetMesh() forces the file to be loaded.

    imported->GetMesh();
    EXPECT_FALSE(imported->WasLoadedSuccessfully());
    EXPECT_TRUE(imported->GetErrorMessage().contains(
                    "Import path was never set"));

    // This causes the ImportedModel to load the shape.
    imported->SetPath(GetDataPath("box.stl").ToString());
    imported->GetMesh();
    EXPECT_TRUE(imported->WasLoadedSuccessfully());
    EXPECT_TRUE(imported->GetErrorMessage().empty());

    imported->SetPath("");
    imported->GetMesh();
    EXPECT_TRUE(imported->WasLoadedSuccessfully());  // Once true, stays true.
    EXPECT_TRUE(imported->GetErrorMessage().contains(
                    "Import path was never set"));

    imported->SetPath("nosuchfile.stl");
    imported->GetMesh();
    EXPECT_TRUE(imported->WasLoadedSuccessfully());
    EXPECT_TRUE(imported->GetErrorMessage().contains("Unable to open"));

    // Test mesh validation for error cases.
    imported->SetPath(GetDataPath("unclosed.stl").ToString());
    EXPECT_ENUM_EQ(MeshValidityCode::kNotClosed,
                   ValidateTriMesh(imported->GetMesh()));

    imported->SetPath(GetDataPath("intersecting.stl").ToString());
    EXPECT_ENUM_EQ(MeshValidityCode::kSelfIntersecting,
                   ValidateTriMesh(imported->GetMesh()));
}
