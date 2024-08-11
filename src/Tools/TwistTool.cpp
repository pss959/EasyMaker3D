//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

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
