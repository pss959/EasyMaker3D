#include "Math/Types.h"
#include "SG/Intersector.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/String.h"

/// \ingroup Tests
class IntersectorTest : public SceneTestBase {
  protected:
    /// Derived Node class that uses a bounds proxy.
    class ProxyNode : public SG::Node {
      protected:
        ProxyNode() {}
        virtual void CreationDone() override {
            SG::Node::CreationDone();
            SetUseBoundsProxy(true);
        }
        friend class Parser::Registry;
    };

    /// Reads a scene from a string and intersects it with the given ray.
    SG::Hit IntersectScene(const Str &input, const Ray &ray) {
        ResetContext();  // Avoid context pollution.
        // Call SetUpIon() so that meshes are installed in some shapes.
        SG::ScenePtr scene = ReadScene(input, true);
        return SG::Intersector::IntersectScene(*scene, ray);
    }

    /// Reads a scene with the given root node contents from a string and
    /// intersects it with the given ray.
    SG::Hit IntersectContents(const Str &contents, const Ray &ray) {
        ResetContext();  // Avoid context pollution.
        // Call SetUpIon() so that meshes are installed in some shapes.
        SG::ScenePtr scene = BuildAndReadScene(contents, true);
        return SG::Intersector::IntersectScene(*scene, ray);
    }

    /// Reads a scene from a string and intersects the graph rooted by the named
    /// Node with the given ray. The \p bounds_only flag is passed to
    /// SG::Intersector::IntersectGraph().
    SG::Hit IntersectGraph(const Str &input, const Str &name, const Ray &ray,
                           bool bounds_only = false) {
        ResetContext();  // Avoid context pollution.
        SG::ScenePtr scene = ReadScene(input, true);
        auto node = SG::FindNodeInScene(*scene, name);
        return SG::Intersector::IntersectGraph(node, ray, bounds_only);
    }
};

TEST_F(IntersectorTest, DefaultHit) {
    const SG::Hit hit0;
    const SG::Hit hit1;
    SG::Hit       hit2;

    // Default values.
    EXPECT_FALSE(hit0.IsValid());
    EXPECT_TRUE(hit0.path.empty());
    EXPECT_NULL(hit0.shape);
    EXPECT_EQ(Ray(),                hit0.world_ray);
    EXPECT_EQ(0,                    hit0.distance);
    EXPECT_EQ(Point3f::Zero(),      hit0.point);
    EXPECT_EQ(Vector3f::Zero(),     hit0.normal);
    EXPECT_EQ(Vector3i(-1, -1, -1), hit0.indices);
    EXPECT_EQ(Vector3f::Zero(),     hit0.barycentric);
    EXPECT_EQ(Point3f::Zero(),      hit0.bounds_point);

    // Equality and inequality.
    hit2.point.Set(0, 0, 1);
    EXPECT_EQ(hit0, hit0);
    EXPECT_EQ(hit0, hit1);
    EXPECT_NE(hit0, hit2);
}

TEST_F(IntersectorTest, EmptyScene) {
    const SG::Hit hit = IntersectContents("", Ray(Point3f(0, 20, 0),
                                                  Vector3f(0, 0, -1)));
    EXPECT_FALSE(hit.IsValid());
    EXPECT_TRUE(hit.path.empty());
    EXPECT_NULL(hit.shape);
}

TEST_F(IntersectorTest, Bounds) {
    const Str input = ReadDataFile("Shapes.emd");

    // Intersect from front. Sphere is at (-100,0,0) with radius 5. The bounds
    // surround it.
    SG::Hit hit = IntersectGraph(input, "Primitives",
                                 Ray(Point3f(-101, 0, 20), Vector3f(0, 0, -1)),
                                 true);
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NULL(hit.shape);
    EXPECT_NEAR(15.f, hit.distance, kClose);  // Sphere has radius 5.
    EXPECT_PTS_CLOSE(Point3f(-1, 0, 5),   hit.point);
    EXPECT_PTS_CLOSE(Point3f(-101, 0, 5), hit.GetWorldPoint());
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1),  hit.normal);
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1),  hit.GetWorldNormal());
}

TEST_F(IntersectorTest, Sphere) {
    const Str input = ReadDataFile("Shapes.emd");

    // Intersect from front. Sphere is at (-100,0,0) with radius 5.
    SG::Hit hit = IntersectGraph(input, "Primitives",
                                 Ray(Point3f(-100, 0, 20), Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
    EXPECT_NEAR(15.f, hit.distance, kClose);  // Sphere has radius 5.
    EXPECT_PTS_CLOSE(Point3f(0, 0, 5),    hit.point);
    EXPECT_PTS_CLOSE(Point3f(-100, 0, 5), hit.GetWorldPoint());
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1),  hit.normal);
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1),  hit.GetWorldNormal());

    // Intersect from bottom.
    hit = IntersectScene(input, Ray(Point3f(-100, -10, 0), Vector3f(0, 1, 0)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_NOT_NULL(hit.shape);
    EXPECT_NEAR(5.f, hit.distance, kClose);  // Sphere has radius 5.
    EXPECT_PTS_CLOSE(Point3f(0, -5, 0),    hit.point);
    EXPECT_PTS_CLOSE(Point3f(-100, -5, 0), hit.GetWorldPoint());
    EXPECT_VECS_CLOSE(Vector3f(0, -1, 0),  hit.normal);
    EXPECT_VECS_CLOSE(Vector3f(0, -1, 0),  hit.GetWorldNormal());

    // Just miss the sphere, but hit the bounds..
    hit = IntersectScene(input, Ray(Point3f(-104.5f, 4.5f, 20),
                                    Vector3f(0, 0, -1)));
    EXPECT_FALSE(hit.IsValid());
}

TEST_F(IntersectorTest, Cone) {
    const Str input = ReadDataFile("Shapes.emd");

    // Intersect from front. Cone is at (100,0,0).
    SG::Hit hit = IntersectGraph(input, "Primitives",
                                 Ray(Point3f(100, 0, 20), Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("Cone", hit.shape->GetName());
    EXPECT_NEAR(5.f, hit.distance, kClose);
    EXPECT_PTS_CLOSE(Point3f(0,   0, 15), hit.point);
    EXPECT_PTS_CLOSE(Point3f(100, 0, 15), hit.GetWorldPoint());
    EXPECT_VECS_CLOSE(Normalized(0, 1, 1), hit.normal);
}

TEST_F(IntersectorTest, Torus) {
    const Str input = ReadDataFile("Shapes.emd");

    // Intersect from front. Torus is at origin with outer radius 1.2 and inner
    // radius .2.
    SG::Hit hit = IntersectGraph(input, "Primitives",
                                 Ray(Point3f(0, 0, 20), Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("Torus", hit.shape->GetName());
    EXPECT_NEAR(18.8f, hit.distance, kClose);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 1.2f), hit.point);

    // Pass down through center, missing.
    hit = IntersectGraph(input, "Primitives",
                         Ray(Point3f(0, 20, 0), Vector3f(0, -1, 0)));
    EXPECT_FALSE(hit.IsValid());
}

TEST_F(IntersectorTest, Rectangles) {
    const Str input = ReadDataFile("Rectangles.emd");

    // The scene is translated 10 units in X.
    // Intersect rays from the center to all 5 rectangles.

    const Point3f center(10, 0, 0);

    SG::Hit hit = IntersectScene(input, Ray(center, Vector3f::AxisZ()));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("Front", hit.path.back()->GetName());

    hit = IntersectScene(input, Ray(center, -Vector3f::AxisZ()));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("Back", hit.path.back()->GetName());

    hit = IntersectScene(input, Ray(center, Vector3f::AxisX()));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("Right", hit.path.back()->GetName());

    hit = IntersectScene(input, Ray(center, -Vector3f::AxisX()));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("Left", hit.path.back()->GetName());

    hit = IntersectScene(input, Ray(center, Vector3f::AxisY()));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("Top", hit.path.back()->GetName());

    hit = IntersectScene(input, Ray(center, -Vector3f::AxisY()));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("Bottom", hit.path.back()->GetName());

    // Just miss (but hit the bounds).
    hit = IntersectScene(input, Ray(Point3f(10 + 20.0001f, 0, 0),
                                    -Vector3f::AxisZ()));
    EXPECT_FALSE(hit.IsValid());
}

TEST_F(IntersectorTest, HiddenParent) {
    const Str input = ReadDataFile("Shapes.emd");

    // The parent Ellipsoid is large and encompasses the two child Ellipsoids.
    // The parent should not be intersected, and its translation should not be
    // applied to the children - if it were, the rays would miss them.

    // Child sphere on the left.
    SG::Hit hit = IntersectGraph(input, "HiddenParent",
                                 Ray(Point3f(-2, 0, 0), Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_EQ("Child0", hit.path.back()->GetName());

    // Child sphere on the right.
    hit = IntersectScene(input, Ray(Point3f(2, 0, 0), Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_EQ("Child1", hit.path.back()->GetName());
}

TEST_F(IntersectorTest, TranslatedShapes) {
    // Tests intersection with shapes that have their translation fields set.
    // Because the translation is inside the shape, it is included in the local
    // coordinates.

    const Str input = ReadDataFile("Shapes.emd");

    // Intersect from front:
    //   Box      is at ( 4,0,0).
    //   Cylinder is at (24,0,0).
    //   Sphere   is at (44,0,0).

    SG::Hit hit;

    hit = IntersectGraph(input, "TranslatedShapes",
                         Ray(Point3f(4, 0, 20), Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("TranslatedBox", hit.shape->GetName());
    EXPECT_NEAR(15.f, hit.distance, kClose);  // Box has size 10.
    EXPECT_PTS_CLOSE(Point3f(4, 0, 5),   hit.point);
    EXPECT_PTS_CLOSE(Point3f(4, 0, 5),   hit.GetWorldPoint());
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1), hit.normal);
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1), hit.GetWorldNormal());

    hit = IntersectGraph(input, "TranslatedShapes",
                         Ray(Point3f(24, 0, 20), Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("TranslatedCylinder", hit.shape->GetName());
    EXPECT_NEAR(15.f, hit.distance, kClose);  // Cylinder has radius 5.
    EXPECT_PTS_CLOSE(Point3f(24, 0, 5),  hit.point);
    EXPECT_PTS_CLOSE(Point3f(24, 0, 5), hit.GetWorldPoint());
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1), hit.normal);

    // Hit the top cap of the cylinder from above.
    hit = IntersectGraph(input, "TranslatedShapes",
                         Ray(Point3f(24, 20, 0), Vector3f(0, -1, 0)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("TranslatedCylinder", hit.shape->GetName());
    EXPECT_NEAR(15.f, hit.distance, kClose);  // Cylinder has height 10.
    EXPECT_PTS_CLOSE(Point3f(24, 5, 0),  hit.point);
    EXPECT_PTS_CLOSE(Point3f(24, 5, 0), hit.GetWorldPoint());
    EXPECT_VECS_CLOSE(Vector3f(0, 1, 0), hit.normal);

    // Hit the bottom cap of the cylinder from below.
    hit = IntersectGraph(input, "TranslatedShapes",
                         Ray(Point3f(24, -20, 0), Vector3f(0, 1, 0)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("TranslatedCylinder", hit.shape->GetName());
    EXPECT_NEAR(15.f, hit.distance, kClose);  // Cylinder has height 10.
    EXPECT_PTS_CLOSE(Point3f(24, -5, 0),  hit.point);
    EXPECT_PTS_CLOSE(Point3f(24, -5, 0), hit.GetWorldPoint());
    EXPECT_VECS_CLOSE(Vector3f(0, -1, 0), hit.normal);

    hit = IntersectGraph(input, "TranslatedShapes",
                         Ray(Point3f(44, 0, 20), Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("TranslatedSphere", hit.shape->GetName());
    EXPECT_NEAR(15.f, hit.distance, kClose);  // Sphere has radius 5.
    EXPECT_PTS_CLOSE(Point3f(44, 0, 5),  hit.point);
    EXPECT_PTS_CLOSE(Point3f(44, 0, 5), hit.GetWorldPoint());
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1), hit.normal);

    // This case was failing due to Box checking transformed bounds instead of
    // untransformed bounds.
    hit = IntersectGraph(input, "ExtraBoxTest",
                         Ray(Point3f(0, -3.5f,  20), -Vector3f::AxisZ()));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("TranslatedBox2", hit.shape->GetName());
    EXPECT_NEAR(15.f, hit.distance, kClose);  // Box has size 10.
    EXPECT_PTS_CLOSE(Point3f(0, -3.5f, 5),   hit.point);
    EXPECT_PTS_CLOSE(Point3f(0, -3.5f, 5), hit.GetWorldPoint());
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1), hit.normal);
}

TEST_F(IntersectorTest, Cone2) {
    const Str input = ReadDataFile("Shapes.emd");
    SG::Hit hit;
    hit = IntersectGraph(input, "ConeTest",
                         Ray(Point3f(0, 8, 20), Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("Cone", hit.shape->GetName());

    // The ray should hit the center of the cone, so the Z value is the average
    // of the top and bottom radii (.15).
    EXPECT_PTS_CLOSE(Point3f(0, 0, .15f), hit.point);
    EXPECT_PTS_CLOSE(Point3f(0, 8, .3f),  hit.GetWorldPoint());
}

TEST_F(IntersectorTest, Cone3) {
    // This case was not working due to a bug where the ray-cone intersection
    // code was returning intersections on the part of the geometric cone on
    // the wrong side of the apex.

    // This is a 90-degree cone with the apex at (0,1,0) and the base at Y=-1.
    const Str contents = R"(
  children: [
    Node "Cone" {
      shapes: [
        Cylinder "Cone" {
          height:        2,
          top_radius:    0,
          bottom_radius: 2,
          has_bottom_cap: False,
        }
      ],
    },
  ]
)";
    // Points straight down. Should hit the cone at (1,0,0) / distance = 10.
    const Ray ray(Point3f(1, 10, 0), Vector3f(0, -1, 0));

    SG::Hit hit;
    hit = IntersectContents(contents, ray);
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_EQ("Cone", hit.path.back()->GetName());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("Cone", hit.shape->GetName());
    EXPECT_NEAR(10.f, hit.distance, kClose);  // Sphere has radius 5.
    EXPECT_PTS_CLOSE(Point3f(1, 0, .0), hit.point);
}

TEST_F(IntersectorTest, Cone4) {
    // Test upside-down cone for completeness.
    const Str contents = R"(
  children: [
    Node "Cone" {
      shapes: [
        Cylinder "Cone" {
          height:        2,
          top_radius:    2,
          bottom_radius: 0,
          has_top_cap: False,
        }
      ],
    },
  ]
)";
    SG::Hit hit;
    hit = IntersectContents(contents,
                            Ray(Point3f(0, 0, 20), Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("Cone", hit.shape->GetName());

    EXPECT_PTS_CLOSE(Point3f(0, 0, 1), hit.point);
    EXPECT_PTS_CLOSE(Point3f(0, 0, 1), hit.GetWorldPoint());
}

TEST_F(IntersectorTest, NonIntersectingShapes) {
    const Str contents = R"(
  children: [
    Node {
      shapes: [
        Line     { end0: -1 -1 0, end1: 1 1 0 },
        PolyLine { points: [-1 -1 0, 1 1 0] },
      ],
    },
  ]
)";
    SG::Hit hit;
    hit = IntersectContents(contents,
                            Ray(Point3f(0, 0, 20), Vector3f(0, 0, -1)));
    EXPECT_FALSE(hit.IsValid());
}

TEST_F(IntersectorTest, ImportedShape) {
    Str contents = R"(
  children: [
    Node {
      shapes: [
        ImportedShape "Imp" {
          path: "<PATH>",
          normal_type: "kFaceNormals",
        }
      ],
    },
  ]
)";

    contents = Util::ReplaceString(contents, "<PATH>",
                                   GetDataPath("Shapes/shape.off").ToString());

    SG::Hit hit;
    hit = IntersectContents(contents,
                            Ray(Point3f(0, 0, 20), Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("Imp", hit.shape->GetName());

    EXPECT_PTS_CLOSE(Point3f(0, 0, .5f), hit.point);
    EXPECT_VECS_CLOSE(Vector3f(0, 0, 1), hit.normal);
}

TEST_F(IntersectorTest, ImportedShapeWithProxy) {
    Str contents = R"(
  children: [
    Node {
      shapes: [
        ImportedShape "Imp" {
          path: "<PATH>",
          normal_type: "kFaceNormals",
          proxy_shape: Box "ProxyBox" { size: 2 2 2 },
          use_bounds_proxy: False,
        }
      ],
    },
  ]
)";

    contents = Util::ReplaceString(contents, "<PATH>",
                                   GetDataPath("Shapes/shape.off").ToString());

    // This ray should hit the bounds of the proxy shape Box (which is twice as
    // large as the ImportedShape) but not the ImportedShape itself.
    SG::Hit hit;
    hit = IntersectContents(contents,
                            Ray(Point3f(.8f, 0, 20), Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    ASSERT_NOT_NULL(hit.shape);
    EXPECT_EQ("Imp", hit.shape->GetName());

    // Turn on use_bounds_proxy and there should be no hit, since the node
    // bounds do not include the proxy shape.
    const Str contents2 = Util::ReplaceString(
        contents, "use_bounds_proxy: False", "use_bounds_proxy: True");
    hit = IntersectContents(contents2,
                            Ray(Point3f(.8f, 0, 20), Vector3f(0, 0, -1)));
    EXPECT_FALSE(hit.IsValid());
}

TEST_F(IntersectorTest, BoundsProxy) {
    Parser::Registry::AddType<ProxyNode>("ProxyNode");

    const Str input = R"(
Scene {
  root_node: ProxyNode "Proxy" {
    translation: 10 0 0,
    shapes: [ Ellipsoid "Sphere" { size: 8 8 8 } ],
  },
}
)";

    // This ray should hit the bounds of the ProxyNode but not the Sphere shape
    // inside it. Normally, this would result in a miss, but ProxyNode
    // specifies that the bounds should be used as a proxy for intersections.
    SG::Hit hit;
    hit = IntersectScene(input, Ray(Point3f(13.8f, 3.8f, 20),
                                    Vector3f(0, 0, -1)));
    EXPECT_TRUE(hit.IsValid());
    EXPECT_FALSE(hit.path.empty());
    EXPECT_EQ("Proxy", hit.path.back()->GetName());
    EXPECT_NULL(hit.shape);  // No shape when proxy is used.
}
