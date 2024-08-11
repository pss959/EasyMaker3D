//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/ConvertMirrorExecutor.h"

#include <ion/math/transformutils.h>

#include "Math/Linear.h"
#include "Math/Plane.h"
#include "Math/Types.h"
#include "Models/MirroredModel.h"
#include "Util/Assert.h"

ConvertedModelPtr ConvertMirrorExecutor::CreateConvertedModel(const Str &name) {
    return Model::CreateModel<MirroredModel>(
        name.empty() ? CreateUniqueName("Mirrored") : name);
}

void ConvertMirrorExecutor::InitConvertedModel(ConvertedModel &model,
                                               const SelPath &path,
                                               const SelPath &primary_path) {
    ASSERT(dynamic_cast<MirroredModel *>(&model));
    auto &mm = static_cast<MirroredModel &>(model);

    if (path == primary_path) {
        // Update the translation for the primary selection.
        mm.SetTranslation(mm.GetOperandModel()->GetTranslation());
    }
    else {
        // Convert the local mirroring plane from the primary selection's
        // object coordinates to this model's object coordinates.
        const auto cc = path.GetCoordConv();
        const Plane object_plane =
            TransformPlane(Plane(0, MirroredModel::GetDefaultPlaneNormal()),
                           cc.GetRootToObjectMatrix() *
                           primary_path.GetCoordConv().GetObjectToRootMatrix());
        mm.SetPlaneNormal(object_plane.normal);

        // The MirroredModel uses a plane through its center (in object
        // coordinates), which leaves the Model in the same place. Update the
        // translation to mirror across the real plane. This is done most
        // easily in stage coordinates. Note that the MirroredModel's operand
        // Model maintains its original translation.
        const Plane stage_plane =
            TransformPlane(object_plane, cc.GetObjectToRootMatrix());
        const Point3f center =
            cc.LocalToRoot(Point3f(mm.GetOperandModel()->GetTranslation()));
        mm.TranslateTo(cc.RootToLocal(stage_plane.MirrorPoint(center)));
    }
}
