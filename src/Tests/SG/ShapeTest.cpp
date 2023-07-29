#include "SG/Exception.h"
#include "SG/ImportedShape.h"
#include "SG/Line.h"
#include "SG/PolyLine.h"
#include "SG/Polygon.h"
#include "SceneTestBase.h"

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

TEST_F(ShapeTest, Line) {
    auto line = CreateObject<SG::Line>();
    EXPECT_NOT_NULL(line);

    // Default settings.
    EXPECT_EQ(Point3f(0, 0, 0), line->GetEnd0());
    EXPECT_EQ(Point3f(1, 0, 0), line->GetEnd1());
    EXPECT_NOT_NULL(line->SetUpIon().Get());

    line->SetEndpoints(Point3f(-2, -3, -4), Point3f(2, 3, 4));
    EXPECT_EQ(Point3f(-2, -3, -4), line->GetEnd0());
    EXPECT_EQ(Point3f( 2,  3,  4), line->GetEnd1());
}

TEST_F(ShapeTest, PolyLine) {
    auto pline = CreateObject<SG::PolyLine>();
    EXPECT_NOT_NULL(pline);

    // Default settings.
    EXPECT_TRUE(pline->GetPoints().empty());
    EXPECT_NOT_NULL(pline->SetUpIon().Get());

    pline->SetPoints(
        std::vector<Point3f>{ Point3f(-2, -3, -4), Point3f(2, 3, 4) });
    EXPECT_EQ(2U,                  pline->GetPoints().size());
    EXPECT_EQ(Point3f(-2, -3, -4), pline->GetPoints()[0]);
    EXPECT_EQ(Point3f( 2,  3,  4), pline->GetPoints()[1]);

    const float radius              = 10;
    const float degrees_per_segment = 30;
    pline->SetArcPoints(CircleArc(Anglef::FromDegrees(0),
                                  Anglef::FromDegrees(180)),
                        radius, degrees_per_segment);
    // 6 segments (180/30) == 7 points.
    EXPECT_EQ(7U,                         pline->GetPoints().size());
    EXPECT_PTS_CLOSE(Point3f( 10,  0, 0), pline->GetPoints()[0]);
    EXPECT_PTS_CLOSE(Point3f(  0, 10, 0), pline->GetPoints()[3]);
    EXPECT_PTS_CLOSE(Point3f(-10,  0, 0), pline->GetPoints()[6]);
}

TEST_F(ShapeTest, Polygon) {
    auto poly = CreateObject<SG::Polygon>();
    EXPECT_NOT_NULL(poly);

    // Default settings.
    EXPECT_TRUE(poly->GetPoints().empty());
    EXPECT_TRUE(poly->GetBorderCounts().empty());
    EXPECT_NOT_NULL(poly->SetUpIon().Get());

    // Set to a Math Polygon
    const std::vector pts{ Point2f(0, 0), Point2f(1, 0), Point2f(0, 1) };
    poly->SetPolygon(Polygon(pts));
    EXPECT_EQ(3U, poly->GetPoints().size());
    EXPECT_EQ(Point2f(0, 0), poly->GetPoints()[0]);
    EXPECT_EQ(Point2f(1, 0), poly->GetPoints()[1]);
    EXPECT_EQ(Point2f(0, 1), poly->GetPoints()[2]);
    EXPECT_EQ(1U, poly->GetBorderCounts().size());
    EXPECT_EQ(3U, poly->GetBorderCounts()[0]);
}

// XXXX Other shape types.
