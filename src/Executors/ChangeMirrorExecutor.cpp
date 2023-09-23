#include "Executors/ChangeMirrorExecutor.h"

#include "Math/Linear.h"
#include "Models/MirroredModel.h"
#include "Util/Assert.h"

Plane ChangeMirrorExecutor::GetModelPlane(const Model &model) const {
    ASSERT(dynamic_cast<const MirroredModel *>(&model));
    return Plane(0, static_cast<const MirroredModel &>(model).GetPlaneNormal());
}

void ChangeMirrorExecutor::SetModelPlane(Model &model, const Plane &plane,
                                         const SG::CoordConv *cc) const {
    ASSERT(dynamic_cast<MirroredModel *>(&model));
    auto &mm = static_cast<MirroredModel &>(model);

    if (cc) {
        // Convert the plane from stage coordinates into object coordinates.
        const Plane object_plane =
            TransformPlane(plane, cc->GetRootToObjectMatrix());

        // Set the plane in the MirroredModel.
        mm.SetPlaneNormal(object_plane.normal);

        // Translate to the other side of the stage plane.
        const Point3f center =
            cc->LocalToRoot(Point3f(mm.GetOperandModel()->GetTranslation()));
        mm.TranslateTo(cc->RootToLocal(plane.MirrorPoint(center)));
    }
    else {
        mm.SetPlaneNormal(plane.normal);
    }
}
