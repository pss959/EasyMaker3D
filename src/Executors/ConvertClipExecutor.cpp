//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/ConvertClipExecutor.h"

#include <ion/math/transformutils.h>

#include "Math/Linear.h"
#include "Math/Types.h"
#include "Models/ClippedModel.h"
#include "Util/Assert.h"

ConvertedModelPtr ConvertClipExecutor::CreateConvertedModel(const Str &name) {
    return Model::CreateModel<ClippedModel>(
        name.empty() ? CreateUniqueName("Clipped") : name);
}

void ConvertClipExecutor::InitConvertedModel(ConvertedModel &model,
                                             const SelPath &path,
                                             const SelPath &primary_path) {
    // Use the local clipping plane from the primary selection for all
    // secondary selections.
    if (path != primary_path) {
        // Matrix to convert from the primary selection's object coordinates to
        // this model's object coordinates.
        const Matrix4f m = path.GetCoordConv().GetRootToObjectMatrix() *
            primary_path.GetCoordConv().GetObjectToRootMatrix();

        ASSERT(dynamic_cast<ClippedModel *>(&model));
        auto &cm = static_cast<ClippedModel &>(model);
        cm.SetPlane(TransformPlane(ClippedModel::GetDefaultPlane(), m));
    }
}
