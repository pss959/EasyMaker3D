#include "Tools/TextTool.h"

#include "Commands/ChangeTextCommand.h"
#include "Base/Tuning.h"
#include "Managers/CommandManager.h"
#include "Models/TextModel.h"
#include "Panels/TextToolPanel.h"
#include "SG/Search.h"
#include "Util/Assert.h"

bool TextTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<TextModel>(sel);
}

void TextTool::InitPanel() {
    auto model = Util::CastToDerived<TextModel>(GetModelAttachedTo());
    ASSERT(model);
    auto &panel = GetTypedPanel<TextToolPanel>();
    std::string font_name = model->GetFontName();
    if (font_name.empty())
        font_name = TK::k3DFont;
    panel.SetValues(model->GetTextString(), font_name, model->GetCharSpacing());
}

void TextTool::PanelChanged(const std::string &key,
                            ToolPanel::InteractionType type) {
    ASSERT(key == "Apply");
    ASSERT(type == ToolPanel::InteractionType::kImmediate);
    TextToolPanel &panel = GetTypedPanel<TextToolPanel>();

    // Access the TextModel to be able to detect changes. Note that at least
    // one field must have changed for the Apply button to be enabled.
    auto model = Util::CastToDerived<TextModel>(GetModelAttachedTo());
    ASSERT(model);

    auto command = CreateCommand<ChangeTextCommand>();
    command->SetFromSelection(GetSelection());
    if (panel.GetTextString() != model->GetTextString())
        command->SetNewTextString(panel.GetTextString());
    if (panel.GetFontName() != model->GetFontName())
        command->SetNewFontName(panel.GetFontName());
    if (panel.GetCharSpacing() != model->GetCharSpacing())
        command->SetNewCharSpacing(panel.GetCharSpacing());
    GetContext().command_manager->AddAndDo(command);
}
