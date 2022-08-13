#include "Executors/CreateTextExecutor.h"

#include "Commands/CreateTextModelCommand.h"
#include "Models/TextModel.h"

ModelPtr CreateTextExecutor::CreateModel(Command &command) {
    CreateTextModelCommand &ctc =
        GetTypedCommand<CreateTextModelCommand>(command);

    std::string name = ctc.GetResultName();
    if (name.empty()) {
        name = CreateUniqueName("Text");
        ctc.SetResultName(name);
    }

    // Create and initialize the Model. Leave the initial scale at its default
    // value (1).
    TextModelPtr tm = Model::CreateModel<TextModel>(name);
    tm->SetTextString(ctc.GetText());
    InitModelTransform(*tm, ctc);
    AddModelInteraction(*tm);
    SetRandomModelColor(*tm);

    // If the Model was not read from a file, drop it from above.
    if (! ctc.IsValidating())
        AnimateModelPlacement(*tm);

    return tm;
}
