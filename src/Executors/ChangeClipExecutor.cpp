#include "Executors/ChangeClipExecutor.h"

#include "Math/Linear.h"
#include "Models/ClippedModel.h"
#include "Util/Assert.h"

Plane ChangeClipExecutor::GetModelPlane(const Model &model) const {
    ASSERT(dynamic_cast<const ClippedModel *>(&model));
    return static_cast<const ClippedModel &>(model).GetPlane();
}

void ChangeClipExecutor::UpdateModel(Model &model,
                                     const PlaneData &data) const {
    ASSERT(dynamic_cast<ClippedModel *>(&model));
    auto &cm = static_cast<ClippedModel &>(model);

    // Save the current translation without offset compensation.
    const Vector3f trans = cm.GetTranslation() - cm.GetLocalCenterOffset();

    // Convert the new plane from stage coordinates into object coordinates.
    const Plane new_object_plane = TransformPlane(
        TranslatePlane(data.new_stage_plane, cm.GetLocalCenterOffset()),
        data.cc.GetRootToObjectMatrix());

    // Set the plane in the ClippedModel.
    cm.SetPlane(new_object_plane);

    // Compensate for any new centering translation.
    cm.SetTranslation(trans + cm.GetLocalCenterOffset());
}

void ChangeClipExecutor::SetModelPlane(Model &model, const Plane &plane) const {
    ASSERT(dynamic_cast<ClippedModel *>(&model));
    auto &cm = static_cast<ClippedModel &>(model);
    cm.SetPlane(plane);
}
