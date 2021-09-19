#include "Managers/ClipboardManager.h"

#include <algorithm>

void ClipboardManager::StoreCopies(const ModelVec_ &models) {
    // Clone all of the Models in the list.
    models_ = CloneModels_(models);

    // Mark all clones as being in the clipboard.
    for (auto &model: models_)
        model->SetUse(Model::Use::kInClipboard);
}

ClipboardManager::ModelVec_ ClipboardManager::CloneModels_(
    const ModelVec_ &models) {
    ModelVec_ clones;
    std::transform(models.begin(), models.end(), std::back_inserter(clones),
                   [](const ModelPtr &m){ return m->CreateClone(); });
    return clones;
}
