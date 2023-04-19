#include "Executors/ConvertMirrorExecutor.h"

#include <ion/math/transformutils.h>

#include "Math/Linear.h"
#include "Math/Types.h"
#include "Models/MirroredModel.h"
#include "Util/Assert.h"

ConvertedModelPtr ConvertMirrorExecutor::CreateConvertedModel() {
    return Model::CreateModel<MirroredModel>(CreateUniqueName("Mirrored"));
}

void ConvertMirrorExecutor::InitConvertedModel(ConvertedModel &model,
                                               const SelPath &path,
                                               const SelPath &primary_path) {
    // Use the local mirroring plane from the primary selection for all
    // secondary selections.
    if (path != primary_path) {
        // Matrix to convert from the primary selection's object coordinates to
        // this model's object coordinates.
        const Matrix4f m = path.GetCoordConv().GetRootToObjectMatrix() *
            primary_path.GetCoordConv().GetObjectToRootMatrix();

        ASSERT(dynamic_cast<MirroredModel *>(&model));
        auto &cm = static_cast<MirroredModel &>(model);
        cm.SetPlane(TransformPlane(MirroredModel::GetDefaultPlane(), m));
    }
}
