//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tools/TextTool.h"

#include "Commands/ChangeTextCommand.h"
#include "Managers/CommandManager.h"
#include "Models/TextModel.h"
#include "Panels/TextToolPanel.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"

bool TextTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<TextModel>(sel);
}

void TextTool::InitPanel() {
    auto model = std::dynamic_pointer_cast<TextModel>(GetModelAttachedTo());
    ASSERT(model);
    auto &panel = GetTypedPanel<TextToolPanel>();
    Str font_name = model->GetFontName();
    if (font_name.empty())
        font_name = TK::k3DFont;  // LCOV_EXCL_LINE [cannot happen]
    panel.SetValues(model->GetTextString(), font_name, model->GetCharSpacing());
}

void TextTool::PanelChanged(const Str &key, ToolPanel::InteractionType type) {
    ASSERT(key == "Apply");
    ASSERT(type == ToolPanel::InteractionType::kImmediate);
    TextToolPanel &panel = GetTypedPanel<TextToolPanel>();

    // Access the TextModel to be able to detect changes. Note that at least
    // one field must have changed for the Apply button to be enabled.
    auto model = std::dynamic_pointer_cast<TextModel>(GetModelAttachedTo());
    ASSERT(model);

    auto command = Command::CreateCommand<ChangeTextCommand>();
    command->SetFromSelection(GetSelection());
    if (panel.GetTextString() != model->GetTextString())
        command->SetNewTextString(panel.GetTextString());
    if (panel.GetFontName() != model->GetFontName())
        command->SetNewFontName(panel.GetFontName());
    if (panel.GetCharSpacing() != model->GetCharSpacing())
        command->SetNewCharSpacing(panel.GetCharSpacing());
    GetContext().command_manager->AddAndDo(command);
}
