#include "Widgets/StageWidget.h"

void StageWidget::PlacePointTarget(const SG::Hit &hit, bool is_alternate_mode,
                                   const SG::NodePath &stage_path,
                                   Point3f &position, Vector3f &direction,
                                   Dimensionality &snapped_dims) {
    GetTargetPlacement_(hit, stage_path, position, direction);

    // No dimension snapping on the Stage.
    snapped_dims.Clear();
}

void StageWidget::GetTargetPlacement_(const SG::Hit &hit,
                                      const SG::NodePath &stage_path,
                                      Point3f &position, Vector3f &direction) {
    // XXXX Need to deal with precision somehow...
    position = stage_path.ToLocal(hit.GetWorldPoint());
    direction = Vector3f::AxisY();
}
