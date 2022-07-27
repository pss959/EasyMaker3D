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
        auto gen_grid = [&](){
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
    radius_ = radius;

    auto geom = SG::FindNodeUnderNode(*this, "StageGeometry");
    const float geom_radius = .5f * geom->GetBounds().GetSize()[0];

    // Scale the stage so that the grid shows the correct working radius.
    radius_scale_ = geom_radius / radius;
    SetUniformScale(interactive_scale_ * radius_scale_);

    // Scale the stage geometry to compensate for the change in scale due to
    // the new radius; the geometry should stay the same regardless of working
    // radius. (The interactive scale should apply to the geometry, so it is
    // not factored in here.)
    geom->SetUniformScale(radius / geom_radius);

    // Regenerate the grid image.
    ASSERT(grid_image_);
    grid_image_->RegenerateImage();
}

void StageWidget::ApplyScaleChange(float delta) {
    // Undo scaling due to radius.
    SetUniformScale(interactive_scale_);

    // Let the DiscWidget class change the interactive scale.
    DiscWidget::ApplyScaleChange(delta);

    // Save the new interactive scale and apply both scales.
    interactive_scale_ = GetScale()[0];
    SetUniformScale(interactive_scale_ * radius_scale_);
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

    // Convert the hit point into stage coordinates: apply only the scaling due
    // to a change in radius, not the interactive scale.
    position = info.hit.point / radius_scale_;

    // Apply the current precision.
    position[0] = RoundToPrecision(position[0], info.linear_precision);
    position[1] = 0;
    position[2] = RoundToPrecision(position[2], info.linear_precision);
}
