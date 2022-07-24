#include "SG/Polygon.h"

#include <ion/gfxutils/shapeutils.h>

#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"

namespace SG {

void Polygon::AddFields() {
    AddField(points_.Init("points"));
    AddField(border_counts_.Init("border_counts"));

    TriMeshShape::AddFields();
}

void Polygon::SetPolygon(const ::Polygon &polygon) {
    points_        = polygon.GetPoints();
    border_counts_ = polygon.GetBorderCounts();

    if (const auto &ion_shape = GetIonShape()) {
        // Build a new version of the shape.
        ion::gfx::ShapePtr new_shape = CreateSpecificIonShape();

        // Swap attribute and index arrays.
        ion_shape->SetAttributeArray(new_shape->GetAttributeArray());
        ion_shape->SetIndexBuffer(new_shape->GetIndexBuffer());
    }

    ProcessChange(Change::kGeometry, *this);
}

ion::gfx::ShapePtr Polygon::CreateSpecificIonShape() {
    // Make sure there are vertices.
    std::vector<Point2f> points = points_.GetValue();
    if (points.size() < 3U) {
        points.push_back(Point2f(0, 0));
        points.push_back(Point2f(1, 0));
        points.push_back(Point2f(0, 1));
    }

    const ::Polygon polygon(points, border_counts_);
    const TriMesh mesh = BuildPolygonMesh(polygon);
    InstallMesh(mesh);

    // Allocate space for normals and texture coordinates to be generated, and
    // allow changes to be made.
    ion::gfx::ShapePtr ion_shape = TriMeshToIonShape(mesh, true, true, true);
    GenerateFaceNormals(*ion_shape);
    GenerateTexCoords(TexCoordsType::kTexCoordsXY, *ion_shape);
    return ion_shape;
}

}  // namespace SG
