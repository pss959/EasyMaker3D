#include "Math/MeshBuilding.h"
#include "SG/Exception.h"
#include "SG/ImportedShape.h"
#include "SG/Line.h"
#include "SG/MutableTriMeshShape.h"
#include "SG/PolyLine.h"
#include "SG/Polygon.h"
#include "SG/Torus.h"
#include "SG/Tube.h"
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

    // Try each type of tex coord mapping.
    const std::string contents = R"(
ImportedShape {
  path:            "<PATH>",
  tex_coords_type: "<TCT>",
}
)";
    for (const auto tct: Util::EnumValues<SG::TriMeshShape::TexCoordsType>()) {
        const std::string input =
            Util::ReplaceString(Util::ReplaceString(contents, "<PATH>",
                                                    path.ToString()),
                                "<TCT>", Util::EnumName(tct));
        imp = ReadTypedItem<SG::ImportedShape>(input);
        EXPECT_NOT_NULL(imp);
        shape = imp->SetUpIon();
        EXPECT_NOT_NULL(shape.Get());
    }

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

TEST_F(ShapeTest, MutableTriMeshShape) {
    auto mtms = CreateObject<SG::MutableTriMeshShape>();
    EXPECT_TRUE(mtms->GetMesh().points.empty());
    EXPECT_TRUE(mtms->GetMesh().indices.empty());
    EXPECT_NOT_NULL(mtms->SetUpIon().Get());

    // Install a new TriMesh.
    auto mesh = BuildBoxMesh(Vector3f(2, 3, 4));
    mtms->ChangeMesh(mesh);
    EXPECT_EQ(mesh.points,  mtms->GetMesh().points);
    EXPECT_EQ(mesh.indices, mtms->GetMesh().indices);

    // Install a ModelMesh with per-vertex texture coordinates and normals.
    ModelMesh mmesh;
    mmesh.points  = mesh.points;
    mmesh.indices = mesh.indices;
    mmesh.normals.assign(mesh.points.size(),    Vector3f::AxisY());
    mmesh.tex_coords.assign(mesh.points.size(), Point2f(.1f, .2f));
    mtms->ChangeModelMesh(mmesh, false);  // Normal per vertex.
    EXPECT_EQ(mesh.points,  mtms->GetMesh().points);
    EXPECT_EQ(mesh.indices, mtms->GetMesh().indices);

    // Try with per-vertex texture coordinates and per-face normals.
    mmesh.normals.assign(mesh.GetTriangleCount(), Vector3f::AxisY());
    mtms->ChangeModelMesh(mmesh, true);  // Normal per face.
    EXPECT_EQ(mesh.points,  mtms->GetMesh().points);
    EXPECT_EQ(mesh.indices, mtms->GetMesh().indices);

    // Test CopyFrom().
    auto copy = CreateObject<SG::MutableTriMeshShape>();
    copy->CopyFrom(*mtms);
    EXPECT_EQ(mtms->GetMesh().points,  copy->GetMesh().points);
    EXPECT_EQ(mtms->GetMesh().indices, copy->GetMesh().indices);
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
    const std::vector<Point2f> pts{
        Point2f(0, 0), Point2f(1, 0), Point2f(0, 1) };
    poly->SetPolygon(Polygon(pts));
    EXPECT_EQ(3U, poly->GetPoints().size());
    EXPECT_EQ(Point2f(0, 0), poly->GetPoints()[0]);
    EXPECT_EQ(Point2f(1, 0), poly->GetPoints()[1]);
    EXPECT_EQ(Point2f(0, 1), poly->GetPoints()[2]);
    EXPECT_EQ(1U, poly->GetBorderCounts().size());
    EXPECT_EQ(3U, poly->GetBorderCounts()[0]);
}

TEST_F(ShapeTest, Torus) {
    auto torus = CreateObject<SG::Torus>();
    EXPECT_NOT_NULL(torus);

    // Default settings.
    EXPECT_EQ(1,   torus->GetOuterRadius());
    EXPECT_EQ(.1f, torus->GetInnerRadius());
    EXPECT_EQ(20,  torus->GetRingCount());
    EXPECT_EQ(20,  torus->GetSectorCount());
    EXPECT_NOT_NULL(torus->SetUpIon().Get());

    // Set fields.
    torus->SetInnerRadius(.5f);
    torus->SetOuterRadius(4.5f);
    EXPECT_EQ(.5f,  torus->GetInnerRadius());
    EXPECT_EQ(4.5f, torus->GetOuterRadius());

    torus->SetGeometry(2.5f, 10.5f, 30, 16);
    EXPECT_EQ(2.5f,  torus->GetInnerRadius());
    EXPECT_EQ(10.5f, torus->GetOuterRadius());
    EXPECT_EQ(30,  torus->GetRingCount());
    EXPECT_EQ(16,  torus->GetSectorCount());
}

TEST_F(ShapeTest, Tube) {
    auto tube = CreateObject<SG::Tube>();
    EXPECT_NOT_NULL(tube);

    // Default settings.
    EXPECT_TRUE(tube->GetPoints().empty());
    EXPECT_EQ(1, tube->GetDiameter());
    EXPECT_EQ(1, tube->GetTaper());
    EXPECT_EQ(4, tube->GetSideCount());
    EXPECT_NOT_NULL(tube->SetUpIon().Get());

    // Set fields.
    tube->SetEndpoints(Point3f(-3, 0, 0), Point3f(4, 2, 1));
    EXPECT_EQ(2U, tube->GetPoints().size());
    EXPECT_EQ(Point3f(-3, 0, 0), tube->GetPoints()[0]);
    EXPECT_EQ(Point3f( 4, 2, 1), tube->GetPoints()[1]);

    const std::vector<Point3f> pts{
        Point3f(0, 0, 0), Point3f(0, 1, 0), Point3f(1, 1, 0) };
    tube->SetPoints(pts);
    EXPECT_EQ(pts, tube->GetPoints());

    const float radius              = 10;
    const float degrees_per_segment = 30;
    tube->SetArcPoints(CircleArc(Anglef::FromDegrees(0),
                                 Anglef::FromDegrees(180)),
                       radius, degrees_per_segment);
    // 6 segments (180/30) == 7 points.
    EXPECT_EQ(7U,                         tube->GetPoints().size());
    EXPECT_PTS_CLOSE(Point3f( 10,  0, 0), tube->GetPoints()[0]);
    EXPECT_PTS_CLOSE(Point3f(  0, 10, 0), tube->GetPoints()[3]);
    EXPECT_PTS_CLOSE(Point3f(-10,  0, 0), tube->GetPoints()[6]);

    tube->SetTaper(2.5f);
    EXPECT_EQ(2.5f, tube->GetTaper());
}

// XXXX Other shape types.
