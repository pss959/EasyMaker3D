#include "Session/SessionTestBase.h"

#include "Managers/SceneContext.h"
#include "Managers/SelectionManager.h"
#include "Managers/SessionManager.h"
#include "SG/Search.h"
#include "Tests/Testing.h"
#include "Util/FilePath.h"
#include "Util/Tuning.h"

bool SessionTestBase::LoadSession(const std::string &file_name) {
    ASSERT(context.session_manager);
    const FilePath path = FilePath::Join(
        FilePath::Join(FilePath::GetTestDataPath(), "Sessions"),
        file_name + TK::kSessionFileSuffix);
    std::string error;
    EXPECT_TRUE(context.session_manager->LoadSession(path, error)) << error;
    return error.empty();
}

void SessionTestBase::SelectModel(const Model &model) {
    const auto path =
        SG::FindNodePathUnderNode(context.scene_context->root_model, model);
    context.selection_manager->ChangeSelection(Selection(path));
}
