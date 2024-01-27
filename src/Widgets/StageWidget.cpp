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
#include "Util/Tuning.h"

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
        grid_image_ =
            std::dynamic_pointer_cast<SG::ProceduralImage>(tex->GetImage());
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

    // Scale the stage geometry to compensate for the change in scale due to
    // the new radius; the geometry should stay the same size regardless of
    // working radius. (The interactive scale applies also to the geometry, so
    // it is not factored in here.)
    const auto geom_scale = 1.f / scale;
    geom_->SetUniformScale(geom_scale);

    // Adjust the Y translation in the geometry to ensure that the top of the
    // stage is at Y=0. This has to be done only here (when the geometry scale
    // changes).
    const auto y_top = geom_scale * geom_->GetBounds().GetMaxPoint()[1];
    geom_->SetTranslation(Vector3f(0, -y_top, 0));
    std::cerr << "XXXX OT=" << geom_->GetBounds().GetMaxPoint()[1]
              << " TR=" << -y_top << "\n";

    // Regenerate the grid image.
    ASSERT(grid_image_);
    grid_image_->RegenerateImage();
}

void StageWidget::SetScaleAndRotation(float scale, const Anglef &angle) {
    SetUniformScale(scale);
    SetRotationAngle(angle);
}

bool StageWidget::ProcessValuator(float delta) {
    // Let the DiscWidget update the scale.
    DiscWidget::ApplyScaleChange(TK::kStageScrollFactor * delta);
    return true;
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
    position = info.hit.point / radius_scaler_->GetScale()[0];

    // Apply the current precision.
    position[0] = RoundToPrecision(position[0], info.linear_precision);
    position[1] = 0;
    position[2] = RoundToPrecision(position[2], info.linear_precision);
}
