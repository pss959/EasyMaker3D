#include "SceneTestBase.h"
#include "SG/Exception.h"
#include "SG/ImportedShape.h"

class ShapeTest : public SceneTestBase {};

TEST_F(ShapeTest, ImportedShape) {
    auto imp = CreateObject<SG::ImportedShape>();
    EXPECT_NOT_NULL(imp);

    // Default settings.
    EXPECT_FALSE(imp->GetFilePath());
    EXPECT_EQ(SG::ImportedShape::NormalType::kNoNormals, imp->GetNormalType());
    EXPECT_EQ(SG::ImportedShape::TexCoordsType::kNoTexCoords,
              imp->GetTexCoordsType());
    EXPECT_NULL(imp->GetProxyShape());
    EXPECT_FALSE(imp->ShouldUseBoundsProxy());

    // Empty path creates an error when setting up Ion.
    TEST_THROW(imp->SetUpIon(), SG::Exception,
               "Unable to open or read shape file");
}

TEST_F(ShapeTest, ImportedShapeCreateFrom) {
    const FilePath path = GetDataPath("Shapes/shape.off");

    // Create with face normals.
    auto imp = SG::ImportedShape::CreateFrom(
        path.ToString(), SG::ImportedShape::NormalType::kFaceNormals);
    EXPECT_EQ(path, imp->GetFilePath());
    EXPECT_EQ(SG::ImportedShape::NormalType::kFaceNormals,
              imp->GetNormalType());
    EXPECT_EQ(SG::ImportedShape::TexCoordsType::kNoTexCoords,
              imp->GetTexCoordsType());
    EXPECT_NULL(imp->GetProxyShape());
    EXPECT_FALSE(imp->ShouldUseBoundsProxy());
    auto shape = imp->SetUpIon();
    EXPECT_NOT_NULL(shape.Get());

    // Create with vertex normals.
    imp = SG::ImportedShape::CreateFrom(
        path.ToString(), SG::ImportedShape::NormalType::kVertexNormals);
    EXPECT_EQ(path, imp->GetFilePath());
    EXPECT_EQ(SG::ImportedShape::NormalType::kVertexNormals,
              imp->GetNormalType());
    EXPECT_EQ(SG::ImportedShape::TexCoordsType::kNoTexCoords,
              imp->GetTexCoordsType());
    EXPECT_NULL(imp->GetProxyShape());
    EXPECT_FALSE(imp->ShouldUseBoundsProxy());
    shape = imp->SetUpIon();
    EXPECT_NOT_NULL(shape.Get());

    // Bad path creates an error when setting up Ion.
    imp = SG::ImportedShape::CreateFrom(
        "/no/such/path", SG::ImportedShape::NormalType::kFaceNormals);
    TEST_THROW(imp->SetUpIon(), SG::Exception,
               "Unable to open or read shape file");
}

// XXXX Other shape types.
