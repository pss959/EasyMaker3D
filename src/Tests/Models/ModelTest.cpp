#include "Tests/Testing.h"
#include "Math/Dimensionality.h"
#include "Models/BoxModel.h"
#include "Models/CylinderModel.h"
#include "Place/DragInfo.h"
#include "Tests/SceneTestBase.h"
#include "Util/Tuning.h"

/// \ingroup Tests
class ModelTest : public SceneTestBase {};

// ----------------------------------------------------------------------------
// Basic interface.
// ----------------------------------------------------------------------------

TEST_F(ModelTest, Level) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    EXPECT_EQ(0, box->GetLevel());
    EXPECT_FALSE(box->IsTopLevel());
    box->SetLevel(1);
    EXPECT_EQ(1, box->GetLevel());
    EXPECT_TRUE(box->IsTopLevel());
    box->SetLevel(2);
    EXPECT_EQ(2, box->GetLevel());
    EXPECT_FALSE(box->IsTopLevel());
}

TEST_F(ModelTest, Use) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    EXPECT_ENUM_EQ(Model::Use::kNew, box->GetUse());

    for (auto use: Util::EnumValues<Model::Use>()) {
        box->SetUse(use);
        EXPECT_EQ(use, box->GetUse());
    }
}

TEST_F(ModelTest, Status) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    EXPECT_ENUM_EQ(Model::Status::kUnknown, box->GetStatus());

    for (auto status: Util::EnumValues<Model::Status>()) {
        box->SetStatus(status);
        EXPECT_EQ(status, box->GetStatus());
    }
}

TEST_F(ModelTest, IsShown) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    EXPECT_FALSE(box->IsShown());
    box->SetStatus(Model::Status::kUnselected);
    EXPECT_TRUE(box->IsShown());
    box->SetStatus(Model::Status::kPrimary);
    EXPECT_TRUE(box->IsShown());
    box->SetStatus(Model::Status::kSecondary);
    EXPECT_TRUE(box->IsShown());
    box->SetStatus(Model::Status::kHiddenByUser);
    EXPECT_FALSE(box->IsShown());
    box->SetStatus(Model::Status::kAncestorShown);
    EXPECT_FALSE(box->IsShown());
    box->SetStatus(Model::Status::kDescendantShown);
    EXPECT_FALSE(box->IsShown());
}

TEST_F(ModelTest, Selection) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    EXPECT_FALSE(box->IsSelected());
    EXPECT_EQ(0U, box->GetSelectionCount());
    box->SetStatus(Model::Status::kUnselected);
    EXPECT_FALSE(box->IsSelected());
    EXPECT_EQ(0U, box->GetSelectionCount());
    box->SetStatus(Model::Status::kPrimary);
    EXPECT_TRUE(box->IsSelected());
    EXPECT_EQ(1U, box->GetSelectionCount());
    box->SetStatus(Model::Status::kSecondary);
    EXPECT_TRUE(box->IsSelected());
    EXPECT_EQ(2U, box->GetSelectionCount());
    box->SetStatus(Model::Status::kHiddenByUser);
    EXPECT_FALSE(box->IsSelected());
    EXPECT_EQ(2U, box->GetSelectionCount());
    box->SetStatus(Model::Status::kAncestorShown);
    EXPECT_FALSE(box->IsSelected());
    EXPECT_EQ(2U, box->GetSelectionCount());
    box->SetStatus(Model::Status::kDescendantShown);
    EXPECT_FALSE(box->IsSelected());
    EXPECT_EQ(2U, box->GetSelectionCount());
}

// ----------------------------------------------------------------------------
// Naming.
// ----------------------------------------------------------------------------

TEST_F(ModelTest, IsValidName) {
    EXPECT_TRUE(Model::IsValidName("Hello"));
    EXPECT_TRUE(Model::IsValidName("Hello!"));
    EXPECT_FALSE(Model::IsValidName(""));
    EXPECT_FALSE(Model::IsValidName(" Hello"));
    EXPECT_FALSE(Model::IsValidName("Hello "));
}

TEST_F(ModelTest, Name) {
    ModelPtr box  = Model::CreateModel<BoxModel>("TestBox");
    EXPECT_EQ("TestBox", box->GetName());
    EXPECT_TRUE(box->GetBaseName().empty());

    ModelPtr copy = box->CloneTyped<BoxModel>(true);
    EXPECT_EQ("TestBox", copy->GetName());
    EXPECT_EQ("TestBox", copy->GetBaseName());

    box->ChangeModelName("NewBox", true);
    EXPECT_EQ("NewBox", box->GetName());
    EXPECT_TRUE(box->GetBaseName().empty());

    // This should fail because the previous change was marked as a user edit.
    box->ChangeModelName("Failure", false);
    EXPECT_EQ("NewBox", box->GetName());
    EXPECT_TRUE(box->GetBaseName().empty());

    copy = box->CloneTyped<BoxModel>(true);
    EXPECT_EQ("NewBox", copy->GetName());
    EXPECT_EQ("NewBox", copy->GetBaseName());

    copy = copy->CloneTyped<BoxModel>(true);
    EXPECT_EQ("NewBox", copy->GetName());
    EXPECT_EQ("NewBox", copy->GetBaseName());
}

// ----------------------------------------------------------------------------
// Placement.
// ----------------------------------------------------------------------------

TEST_F(ModelTest, MoveTo) {
    ModelPtr box = Model::CreateModel<BoxModel>();
    Vector3f   scale(1, 2, 3);
    Rotationf  rot = Rotationf::FromEulerAngles(Anglef::FromDegrees(20),
                                                Anglef::FromDegrees(30),
                                                Anglef::FromDegrees(-40));
    Vector3f   trans(100, 200, 300);
    box->SetScale(scale);
    box->SetRotation(rot);
    box->SetTranslation(trans);

    // Move the center of the model. Rotation and scale should be untouched.
    Point3f c(-10, 3, 14);
    box->MoveCenterTo(c);
    EXPECT_EQ(scale, box->GetScale());
    EXPECT_EQ(rot,   box->GetRotation());
    EXPECT_EQ(c,     Point3f(box->GetTranslation()));

    // Move the bottom center. This should also change rotation.
    box->MoveBottomCenterTo(c, Vector3f(0, 1, 0));
    EXPECT_EQ(scale, box->GetScale());
    EXPECT_TRUE(box->GetRotation().IsIdentity());
    EXPECT_EQ(c + Vector3f(0, scale[1], 0), Point3f(box->GetTranslation()));
}

// ----------------------------------------------------------------------------
// Complexity handling.
// ----------------------------------------------------------------------------

TEST_F(ModelTest, Complexity) {
    // BoxModel does not respond to complexity, but CylinderModel does.
    ModelPtr box = Model::CreateModel<BoxModel>();
    ModelPtr cyl = Model::CreateModel<CylinderModel>();
    EXPECT_FALSE(box->CanSetComplexity());
    EXPECT_TRUE(cyl->CanSetComplexity());

    EXPECT_EQ(TK::kModelComplexity, box->GetComplexity());
    EXPECT_EQ(TK::kModelComplexity, cyl->GetComplexity());

    box->SetComplexity(.2f);
    cyl->SetComplexity(.2f);
    EXPECT_EQ(TK::kModelComplexity, box->GetComplexity());
    EXPECT_EQ(.2f,                  cyl->GetComplexity());
}

// ----------------------------------------------------------------------------
// Color handling.
// ----------------------------------------------------------------------------

TEST_F(ModelTest, Color) {
    Model::ResetColors();
    auto c0a = Model::GetNextColor();
    auto c1a = Model::GetNextColor();
    auto c2a = Model::GetNextColor();
    EXPECT_NE(c0a, c1a);
    EXPECT_NE(c0a, c2a);
    EXPECT_NE(c1a, c2a);

    Model::ResetColors();
    auto c0b = Model::GetNextColor();
    auto c1b = Model::GetNextColor();
    auto c2b = Model::GetNextColor();
    EXPECT_EQ(c0a, c0b);
    EXPECT_EQ(c1a, c1b);
    EXPECT_EQ(c2a, c2b);

    ModelPtr box = ReadRealNode<BoxModel>("children: [ BoxModel \"Boxy\" {} ]",
                                          "Boxy");
    box->SetColor(Color(0, 1, 0));
    EXPECT_EQ(Color(0, 1, 0), box->GetColor());
}

// ----------------------------------------------------------------------------
// Mesh query.
// ----------------------------------------------------------------------------

TEST_F(ModelTest, Mesh) {
    auto cyl = Model::CreateModel<CylinderModel>();

    const auto m0 = cyl->GetMesh();
    cyl->SetComplexity(0);

    // The mesh should be marked as stale due to the complexity change.
    // GetCurrentMesh() returns the stale mesh, but GetMesh() should return the
    // updated one.
    const auto m1 = cyl->GetCurrentMesh();
    const auto m2 = cyl->GetMesh();

    EXPECT_EQ(m0.points,  m1.points);
    EXPECT_EQ(m0.indices, m1.indices);
    EXPECT_GT(m0.points.size(),  m2.points.size());
    EXPECT_GT(m0.indices.size(), m2.indices.size());
}

TEST_F(ModelTest, Volume) {
    auto box = Model::CreateModel<BoxModel>();

    // Unscaled BoxModel.
    EXPECT_EQ(Vector3f(2, 2, 2), box->GetBounds().GetSize());
    EXPECT_EQ(Vector3f(2, 2, 2), box->GetScaledBounds().GetSize());
    EXPECT_EQ(8,                 box->ComputeVolume());

    // Scaled BoxModel.
    box->SetScale(Vector3f(3, 4, 5));
    EXPECT_EQ(Vector3f(2, 2, 2) , box->GetBounds().GetSize());
    EXPECT_EQ(Vector3f(6, 8, 10), box->GetScaledBounds().GetSize());
    EXPECT_EQ(480,                box->ComputeVolume());
}

// ----------------------------------------------------------------------------
// Update.
// ----------------------------------------------------------------------------

TEST_F(ModelTest, UpdateForRenderPass) {
    // Need a real node with uniforms set up.
    auto box = ReadRealNode<BoxModel>(
        R"(children: [ BoxModel "Boxy" {} ])", "Boxy");
    box->UpdateForRenderPass("Shadow");
}

// ----------------------------------------------------------------------------
// Target interface.
// ----------------------------------------------------------------------------

TEST_F(ModelTest, PlacePointTarget) {
    auto stage = CreateObject<SG::Node>();
    auto box   = Model::CreateModel<BoxModel>();
    box->SetUniformScale(8);
    stage->AddChild(box);

    // This is required to get the BoxModel to update matrices.
    box->SetStatus(Model::Status::kUnselected);

    DragInfo info;
    info.path_to_stage  = SG::NodePath(stage);
    info.path_to_widget = SG::NodePath(stage);
    info.path_to_widget.push_back(box);
    info.hit.path = info.path_to_widget;

    // Note that hit points are in object coordinates and expected target
    // points are in stage coordinates (scaled by the BoxModel scale).  Also
    // note that the object bounds range from -1 to 1 in all dimensions.
    auto test_pt = [&](const Point3f &p, const Vector3f &n, bool use_bounds,
                       const Point3f &exp_pos, const Vector3f &exp_dir,
                       const Str &exp_dims){
        info.is_modified_mode = use_bounds;
        info.hit.point        = p;
        info.hit.bounds_point = p;
        info.hit.normal       = n;

        Point3f        target_pos;
        Vector3f       target_dir;
        Dimensionality snapped_dims;
        box->PlacePointTarget(info, target_pos, target_dir, snapped_dims);
        EXPECT_EQ(exp_dims, snapped_dims.GetAsString());
        EXPECT_EQ(exp_pos,  target_pos);
        EXPECT_EQ(exp_dir,  target_dir);
    };

    // Points on mesh. ------------------------

    // Point too far from any vertex.
    test_pt(Point3f(.4f,  .6f,  1), Vector3f(0, 0, 1), false,
            Point3f(3.2f, 4.8f, 8), Vector3f(0, 0, 1), "");

    // Point close enough to vertex.
    test_pt(Point3f(.99f, .99f,     1), Vector3f(0,  0, 1), false,
            Point3f(8,       8,     8), Vector3f(0,  0, 1), "XYZ");
    test_pt(Point3f(-1,  -.99f, -.99f), Vector3f(-1, 0, 0), false,
            Point3f(-8,     -8,    -8), Vector3f(-1, 0, 0), "XYZ");

    // Points on bounds. ------------------------

    // Point far enough from the middle or corner of bounds.
    test_pt(Point3f(.2f,    .7f, 1), Vector3f(0,  0, 1), true,
            Point3f(1.6f,  5.6f, 8), Vector3f(0,  0, 1), "Z");

    // Points close to the middle and edges of bounds.
    test_pt(Point3f(.01f, .99f,     1), Vector3f(0,  0, 1), true,
            Point3f(0,       8,     8), Vector3f(0,  0, 1), "XYZ");
    test_pt(Point3f(1,   -.99f, -.01f), Vector3f(1,  0, 0), true,
            Point3f(8,     -8,      0), Vector3f(1,  0, 0), "XYZ");
}

TEST_F(ModelTest, PlaceEdgeTarget) {
    auto stage = CreateObject<SG::Node>();
    auto box   = Model::CreateModel<BoxModel>();
    box->SetUniformScale(8);
    stage->AddChild(box);

    // This is required to get the BoxModel to update matrices and mesh.
    box->SetStatus(Model::Status::kUnselected);
    const auto mesh = box->GetMesh();

    DragInfo info;
    info.path_to_stage  = SG::NodePath(stage);
    info.path_to_widget = SG::NodePath(stage);
    info.path_to_widget.push_back(box);
    info.hit.path = info.path_to_widget;

    // Note that hit points are in object coordinates and expected target
    // points are in stage coordinates (scaled by the BoxModel scale).  Also
    // note that the object bounds range from -1 to 1 in all dimensions.  The
    // tri_index and opposite_vert_index are unused when placing edges on
    // bounds.
    auto test_edge = [&](const Point3f &p, size_t tri_index,
                         size_t opposite_vert_index, bool use_bounds,
                         const Point3f &exp_pos0, const Point3f &exp_pos1){
        info.is_modified_mode = use_bounds;
        info.hit.bounds_point = p;
        // Set the indices of the selected triangle.
        for (int i = 0; i < 3; ++i)
            info.hit.indices[i] = mesh.indices[3 * tri_index + i];
        // Set the barycentric coordinates so the correct edge is chosen.
        info.hit.barycentric.Set(.5f, .5f, .5f);
        info.hit.barycentric[opposite_vert_index] = 0;

        float   target_len = 1000;  // Unused.
        Point3f target_pos0;
        Point3f target_pos1;
        box->PlaceEdgeTarget(info, target_len, target_pos0, target_pos1);
        EXPECT_EQ(exp_pos0, target_pos0);
        EXPECT_EQ(exp_pos1, target_pos1);
    };

    // Edges of mesh. ------------------------

    // Top front edge (point index 3 to 7).
    test_edge(Point3f(.1f, .9f, 1), 3, 2, false,
              Point3f(-8, 8, 8), Point3f(8, 8, 8));

    // Left side bottom-front to top-back diagonal edge (point index 1 to 2).
    test_edge(Point3f(-1, .01f, -.01f), 0, 0, false,
              Point3f(-8, -8, 8), Point3f(-8, 8, -8));

    // Edges of bounds. ------------------------

    // Top edge of bounds on front face.
    test_edge(Point3f(.1f, .9f, 1), 0, 0, true,
              Point3f(-8, 8, 8), Point3f(8, 8, 8));
    // Back edge of bounds on right face.
    test_edge(Point3f(1, .3f, -.9f), 0, 0, true,
              Point3f(8, -8, -8), Point3f(8, 8, -8));
    // Left edge of bounds on bottom face.
    test_edge(Point3f(-.9f, -1, -.1f), 0, 0, true,
              Point3f(-8, -8, -8), Point3f(-8, -8, 8));
}
