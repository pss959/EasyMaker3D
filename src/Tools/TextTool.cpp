#include "Tools/TextTool.h"

#include "Commands/ChangeTextCommand.h"
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
    GetTypedPanel<TextToolPanel>().SetTextString(model->GetTextString());
    // XXXX Other parts of panel.
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
