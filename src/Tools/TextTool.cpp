#include "Tools/TextTool.h"

#include "Commands/ChangeTextCommand.h"
#include "Defaults.h"
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
        font_name = Defaults::kFontName;
    panel.SetValues(model->GetTextString(), font_name, model->GetCharSpacing());
}

void TextTool::PanelChanged(const std::string &key,
                            ToolPanel::InteractionType type) {
#if XXXX
    // XXXX
    ASSERT(key == "Text");
    ASSERT(type == ToolPanel::InteractionType::kImmediate);

    TextToolPanel &panel = GetTypedPanel<TextToolPanel>();

    auto command = CreateCommand<ChangeTextCommand>();
    command->SetFromSelection(GetSelection());
    command->SetNewText(panel.GetText());
    GetContext().command_manager->AddAndDo(command);
#endif
}
