//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Math/Bounds.h"
#include "Math/MeshBuilding.h"
#include "Math/MeshClipping.h"
#include "Math/MeshUtils.h"
#include "Math/Plane.h"
#include "Math/TriMesh.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class MeshClippingTest : public TestBase {};

TEST_F(MeshClippingTest, ClipMesh) {
    // Build a 10x10x10 box centered on the origin and clip off the top half.
    TriMesh box = BuildBoxMesh(Vector3f(10, 10, 10));
    TriMesh clipped = ClipMesh(box, Plane(0, Vector3f::AxisY()));

    Bounds bounds = ComputeMeshBounds(clipped);
    EXPECT_EQ(Vector3f(10, 5, 10), bounds.GetSize());
    EXPECT_EQ(Point3f(0, -2.5f, 0), bounds.GetCenter());
}
