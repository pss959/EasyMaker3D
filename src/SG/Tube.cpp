#include "SG/Tube.h"

#include "Math/MeshBuilding.h"
#include "Math/MeshUtils.h"
#include "Util/General.h"

namespace SG {

void Tube::AddFields() {
    AddField(points_.Init("points"));
    AddField(diameter_.Init("diameter",     1));
    AddField(side_count_.Init("side_count", 4));

    TriMeshShape::AddFields();
}

void Tube::SetPoints(const std::vector<Point3f> &points) {
    points_ = points;
    UpdateIonShape_();
    ProcessChange(Change::kGeometry, *this);
}

void Tube::SetArcPoints(const CircleArc &arc, float radius,
                            float degrees_per_segment) {
    std::vector<Point3f> points;
    const int seg_count = static_cast<int>(std::abs(arc.arc_angle.Degrees()) /
                                           degrees_per_segment);
    if (seg_count) {
        points = Util::ConvertVector<Point3f, Point2f>(
            GetCircleArcPoints(1 + seg_count, radius, arc),
            [](const Point2f &p){ return Point3f(p, 0); });
    }
    SetPoints(points);
}

ion::gfx::ShapePtr Tube::CreateSpecificIonShape() {
    const TriMesh mesh = BuildMesh_();
    InstallMesh(mesh);

    // Allocate space for normals to be generated.
    ion::gfx::ShapePtr ion_shape = TriMeshToIonShape(mesh, true, false);
    GenerateVertexNormals(*ion_shape);
    return ion_shape;
}

void Tube::UpdateIonShape_() {
    if (ion::gfx::ShapePtr ion_shape = GetIonShape()) {
        const TriMesh mesh = BuildMesh_();
        InstallMesh(mesh);
        UpdateIonShapeFromTriMesh(mesh, *ion_shape, true, false);
        GenerateVertexNormals(*ion_shape);
    }
}

TriMesh Tube::BuildMesh_() const {
    return BuildTubeMesh(points_, diameter_, side_count_);
}

}  // namespace SG
