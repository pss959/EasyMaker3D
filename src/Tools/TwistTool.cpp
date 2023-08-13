#include "Tools/TwistTool.h"

#include "Commands/ChangeTwistCommand.h"
#include "Models/TwistedModel.h"

bool TwistTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<TwistedModel>(sel);
}

Spin TwistTool::GetObjectSpinFromModel() const {
    auto bm = std::dynamic_pointer_cast<TwistedModel>(GetModelAttachedTo());
    ASSERT(bm);
    return bm->GetSpin();
}

ChangeSpinCommandPtr TwistTool::CreateChangeSpinCommand() const {
    return Command::CreateCommand<ChangeTwistCommand>();
}
