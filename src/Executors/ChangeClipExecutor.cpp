#include "Executors/ChangeClipExecutor.h"

#include "Math/Linear.h"
#include "Models/ClippedModel.h"
#include "Util/Assert.h"

Plane ChangeClipExecutor::GetModelPlane(const Model &model) const {
    ASSERT(dynamic_cast<const ClippedModel *>(&model));
    return static_cast<const ClippedModel &>(model).GetPlane();
}

void ChangeClipExecutor::SetModelPlane(Model &model, const Plane &plane,
                                       const SG::CoordConv *cc) const {
    ASSERT(dynamic_cast<ClippedModel *>(&model));
    auto &cm = static_cast<ClippedModel &>(model);

    if (cc) {
        // Save the current translation without offset compensation.
        const Vector3f trans = cm.GetTranslation() - cm.GetLocalCenterOffset();

        // Convert the plane from stage coordinates into object coordinates.
        const Plane object_plane = TransformPlane(
            TranslatePlane(plane, cm.GetLocalCenterOffset()),
            cc->GetRootToObjectMatrix());

        // Set the plane in the ClippedModel.
        cm.SetPlane(object_plane);

        // Compensate for any new centering translation.
        cm.SetTranslation(trans + cm.GetLocalCenterOffset());
    }
    else {
        cm.SetPlane(plane);
    }
}
