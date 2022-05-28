#include "Widgets/StageWidget.h"

#include "App/DragInfo.h"
#include "Base/Procedural.h"
#include "Math/Linear.h"
#include "SG/ColorMap.h"
#include "SG/Node.h"
#include "SG/ProceduralImage.h"
#include "SG/Search.h"
#include "SG/Texture.h"
#include "SG/UniformBlock.h"
#include "Util/Assert.h"
#include "Util/General.h"

void StageWidget::CreationDone() {
    DiscWidget::CreationDone();
    if (! IsTemplate()) {
        // Set up the function to draw the grid.
        auto gen_grid = [&]{
            const Color x_color = SG::ColorMap::SGetColorForDimension(0);
            const Color y_color = SG::ColorMap::SGetColorForDimension(1);
            return GenerateGridImage(radius_, x_color, y_color);
        };

        // Access the ProceduralImage from the Texture from the UniformBlock
        // and set its function.
        auto geom = SG::FindNodeUnderNode(*this, "StageGeometry");
        ASSERT(! geom->GetUniformBlocks().empty());
        const auto &block = geom->GetUniformBlocks()[0];
        ASSERT(block);
        ASSERT(! block->GetTextures().empty());
        const auto &tex = block->GetTextures()[0];
        ASSERT(tex);
        grid_image_ = Util::CastToDerived<SG::ProceduralImage>(tex->GetImage());
        ASSERT(grid_image_);
        grid_image_->SetFunction(gen_grid);
    }
}

void StageWidget::SetStageRadius(float radius) {
    ASSERT(radius > 0);

    // Get the size of the stage geometry.
    const float geom_radius = .5f * GetBounds().GetSize()[0];
    ASSERT(geom_radius > 0);

    // Scale everything on the stage (except the geometry) to compensate for
    // the change in radius.
    auto mh = SG::FindNodeUnderNode(*this, "ModelHider");
    const float scale = geom_radius / radius;
    mh->SetUniformScale(scale);

    // Save the radius and scale.
    radius_       = radius;
    radius_scale_ = scale;

    // Regenerate the grid image.
    ASSERT(grid_image_);
    grid_image_->RegenerateImage();
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

    // Convert the hit point into stage coordinates and apply precision.
    position  = info.hit.point / radius_scale_;
    position[0] = RoundToPrecision(position[0], info.linear_precision);
    position[1] = 0;
    position[2] = RoundToPrecision(position[2], info.linear_precision);
}
