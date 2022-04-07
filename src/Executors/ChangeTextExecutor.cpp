#include "Executors/ChangeTextExecutor.h"

#include "Commands/ChangeTextCommand.h"
#include "Models/TextModel.h"

void ChangeTextExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    ChangeTextCommand &ctc = GetTypedCommand<ChangeTextCommand>(command);

    if (operation == Command::Op::kDo) {
        for (auto &pm: data.per_model) {
            TextModel &tm = GetTypedModel<TextModel>(pm.path_to_model);
            if (! ctc.GetNewTextString().empty())
                tm.SetTextString(ctc.GetNewTextString());
            if (! ctc.GetNewFontName().empty())
                tm.SetFontName(ctc.GetNewFontName());
            if (ctc.GetNewCharSpacing() != 1)
                tm.SetCharSpacing(ctc.GetNewCharSpacing());
        }
    }
    else {  // Undo.
        for (auto &pm: data.per_model) {
            TextModel &tm = GetTypedModel<TextModel>(pm.path_to_model);
            tm.SetTextString(pm.old_text_string);
            tm.SetFontName(pm.old_font_name);
            tm.SetCharSpacing(pm.old_char_spacing);
        }
    }

    // There is no need to reselect for a Panel-based Tool.
}

ChangeTextExecutor::ExecData_ & ChangeTextExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeTextCommand &ctc = GetTypedCommand<ChangeTextCommand>(command);

        const auto &model_names = ctc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());
        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            pm.path_to_model  = FindPathToModel(model_names[i]);
            TextModel &tm = GetTypedModel<TextModel>(pm.path_to_model);
            pm.old_text_string  = tm.GetTextString();
            pm.old_font_name    = tm.GetFontName();
            pm.old_char_spacing = tm.GetCharSpacing();
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
