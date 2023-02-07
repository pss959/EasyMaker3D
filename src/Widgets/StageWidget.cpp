#include "Widgets/StageWidget.h"

#include "Base/Procedural.h"
#include "Math/Linear.h"
#include "Place/DragInfo.h"
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
        // Find the important nodes.
        radius_scaler_ = SG::FindNodeUnderNode(*this, "RadiusScaler");
        geom_          = SG::FindNodeUnderNode(*this, "StageGeometry");

        // Set up the function to draw the grid.
        auto gen_grid = [&](){
            // Use X/Z indices here to get the correct colors.
            const Color x_color = SG::ColorMap::SGetColorForDimension(0);
            const Color y_color = SG::ColorMap::SGetColorForDimension(2);
            return GenerateGridImage(radius_, x_color, y_color);
        };

        // Access the ProceduralImage from the Texture from the UniformBlock
        // and set its function.
        ASSERT(! geom_->GetUniformBlocks().empty());
        const auto &block = geom_->GetUniformBlocks()[0];
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

    // Scale everything so that the grid shows the correct working radius.
    const float geom_radius = .5f * geom_->GetBounds().GetSize()[0];
    const float scale = geom_radius / radius;
    radius_scaler_->SetUniformScale(scale);

    // Scale the stage geometry in X and Z to compensate for the change in
    // scale due to the new radius; the geometry should stay the same
    // regardless of working radius. (The interactive scale should apply to the
    // geometry, so it is not factored in here.)
    Vector3f geom_scale = geom_->GetScale();
    geom_scale[0] = geom_scale[2] = 1.f / scale;
    geom_->SetScale(geom_scale);

    // Adjust the Y scale in the geometry to maintain a constant size.
    FixGeometryYScale_();

    // Regenerate the grid image.
    ASSERT(grid_image_);
    grid_image_->RegenerateImage();
}

void StageWidget::ApplyScaleChange(float delta) {
    // Let the DiscWidget update the scale.
    DiscWidget::ApplyScaleChange(delta);

    // Adjust the Y scale in the geometry to maintain a constant size.
    FixGeometryYScale_();
}

void StageWidget::SetScaleAndRotation(float scale, const Anglef &angle) {
    SetUniformScale(scale);
    SetRotationAngle(angle);
    FixGeometryYScale_();
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

void StageWidget::FixGeometryYScale_() {
    // Apply the inverse of the scale in Y to the geometry to maintain a
    // constant size of 1. Note that the scale includes both the StageWidget
    // scale and the radius scale.
    const float stage_y_scale = GetScale()[1] * radius_scaler_->GetScale()[1];
    Vector3f geom_scale = geom_->GetScale();
    geom_scale[1] = 1.f / stage_y_scale;
    geom_->SetScale(geom_scale);
}

void StageWidget::GetTargetPlacement_(const DragInfo &info,
                                      Point3f &position, Vector3f &direction) {
    direction = Vector3f::AxisY();

    // Convert the hit point into stage coordinates: apply only the scaling due
    // to a change in radius, not the interactive scale.
    position = info.hit.point / radius_scaler_->GetScale()[0];

    // Apply the current precision.
    position[0] = RoundToPrecision(position[0], info.linear_precision);
    position[1] = 0;
    position[2] = RoundToPrecision(position[2], info.linear_precision);
}
