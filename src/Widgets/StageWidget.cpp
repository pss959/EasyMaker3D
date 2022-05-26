#include "Widgets/StageWidget.h"

#include "App/DragInfo.h"
#include "Math/Linear.h"
#include "SG/Node.h"
#include "SG/ProceduralImage.h"
#include "SG/Search.h"
#include "SG/Texture.h"
#include "SG/UniformBlock.h"

void StageWidget::CreationDone() {
    DiscWidget::CreationDone();
    if (! IsTemplate()) {
        // Get the size of the geometry to set up the scaling ratio.
        std::cerr << "XXXX Stage radius = "
                  << (.5f * GetBounds().GetSize()[0]) << "\n";
    }
}

SG::ProceduralImagePtr StageWidget::GetGridImage() const {
    // Access the Image from the Texture from the UniformBlock.

    auto geom = SG::FindNodeUnderNode(*this, "StageGeometry");
    ASSERT(! geom->GetUniformBlocks().empty());
    const auto &block = geom->GetUniformBlocks()[0];
    ASSERT(block);
    ASSERT(! block->GetTextures().empty());
    const auto &tex = block->GetTextures()[0];
    ASSERT(tex);
    const auto &im = Util::CastToDerived<SG::ProceduralImage>(tex->GetImage());
    ASSERT(im);
    return im;
}

void StageWidget::PlacePointTarget(const DragInfo &info,
                                   Point3f &position, Vector3f &direction,
                                   Dimensionality &snapped_dims) {
    GetTargetPlacement_(info, position, direction);

    // No dimension snapping on the Stage.
    snapped_dims.Clear();
}

void StageWidget::PlaceEdgeTarget(const DragInfo &info, float current_length,
                                  Point3f &position0, Point3f &position1) {
    Vector3f direction;
    GetTargetPlacement_(info, position0, direction);
    position1 = position0 + current_length * direction;
}

void StageWidget::GetTargetPlacement_(const DragInfo &info,
                                      Point3f &position, Vector3f &direction) {
    direction = Vector3f::AxisY();

    // The hit point is already in the object coordinates of the stage, which
    // are the same as stage coordinates.
    position  = info.hit.point;
    position[0] = RoundToPrecision(position[0], info.linear_precision);
    position[1] = 0;
    position[2] = RoundToPrecision(position[2], info.linear_precision);
}
