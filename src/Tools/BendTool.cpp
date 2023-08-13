#include "Tools/BendTool.h"

#include "Commands/ChangeBendCommand.h"
#include "Models/BentModel.h"

bool BendTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<BentModel>(sel);
}

Spin BendTool::GetObjectSpinFromModel() const {
    auto bm = std::dynamic_pointer_cast<BentModel>(GetModelAttachedTo());
    ASSERT(bm);
    return bm->GetSpin();
}

ChangeSpinCommandPtr BendTool::CreateChangeSpinCommand() const {
    return Command::CreateCommand<ChangeBendCommand>();
}
