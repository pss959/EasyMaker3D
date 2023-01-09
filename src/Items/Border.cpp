#include "Items/Border.h"

#include "Base/Tuning.h"
#include "SG/MutableTriMeshShape.h"
#include "Util/Assert.h"

Border::Border() {
}

void Border::AddFields() {
    AddField(color_.Init("color", Color::Black()));
    AddField(width_.Init("width", 1));

    SG::Node::AddFields();
}

void Border::PostSetUpIon() {
    SG::Node::PostSetUpIon();
    UpdateMesh_();
}

void Border::SetColor(const Color &color) {
    color_ = color;
    UpdateMesh_();
}

void Border::SetWidth(float width) {
    width_ = width;
    UpdateMesh_();
}

void Border::SetSize(const Vector2f &size) {
    size_ = size;
    UpdateMesh_();
}

void Border::UpdateMesh_() {
    // Don't do anything until Ion is set up.
    if (! GetIonNode())
        return;

    const float width = width_;

    if (width <= 0) {
        SetEnabled(false);
        return;
    }
    SetEnabled(true);
    SetBaseColor(color_);

    // Set up the TriMesh to form a border with the correct width.
    ASSERT(GetShapes().size() == 1U);
    auto mtms = Util::CastToDerived<SG::MutableTriMeshShape>(GetShapes()[0]);
    ASSERT(mtms);

    TriMesh mesh;

    // Outer and inner sizes in x and y. If the size in a dimension is S and
    // the border width is W, then the scaled outer border size is S and the
    // inner size is (S-W). In unscaled coordinates, these are 1 and 1-W/S. Use
    // half on each side of the origin.
    const float w = TK::kBorderBaseWidth * width;
    const float xo = .5f;
    const float xi = .5f * (1 - w / size_[0]);
    const float yo = .5f;
    const float yi = .5f * (1 - w / size_[1]);
    mesh.points.resize(8);
    mesh.points[0].Set(-xo, -yo, 0);
    mesh.points[1].Set( xo, -yo, 0);
    mesh.points[2].Set( xo,  yo, 0);
    mesh.points[3].Set(-xo,  yo, 0);
    mesh.points[4].Set(-xi, -yi, 0);
    mesh.points[5].Set( xi, -yi, 0);
    mesh.points[6].Set( xi,  yi, 0);
    mesh.points[7].Set(-xi,  yi, 0);

    // Points are:
    //
    //    3-------------2
    //    |  7-------6  |
    //    |  |       |  |
    //    |  4-------5  |
    //    0-------------1
    //
    // The indices form 4 trapezoids, each of which is divided into 2 triangles.
    mesh.indices = std::vector<GIndex>{
        0, 4, 7, 0, 7, 3,  // Left.
        0, 1, 5, 0, 5, 4,  // Bottom.
        1, 2, 6, 1, 6, 5,  // Right.
        2, 3, 7, 2, 7, 6,  // Top.
    };

    mtms->ChangeMesh(mesh);
}
