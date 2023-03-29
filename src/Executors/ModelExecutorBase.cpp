#include "Executors/ModelExecutorBase.h"

#include "Commands/CreateModelCommand.h"
#include "Managers/AnimationManager.h"
#include "Managers/CommandManager.h"
#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Managers/TargetManager.h"
#include "Math/Linear.h"
#include "Place/PointTarget.h"
#include "Selection/SelPath.h"
#include "Selection/Selection.h"
#include "Util/General.h"
#include "Util/Tuning.h"

void ModelExecutorBase::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    // Process the command.
    const Context &context = GetContext();
    Selection sel;
    if (operation == Command::Op::kDo) {
        // Add as a top-level Model and select it.
        context.root_model->AddChildModel(data.model);
        sel.Add(SelPath(context.root_model, data.model));
        context.name_manager->Add(data.model->GetName());
    }
    else {  // Undo.
        const int index = context.root_model->GetChildModelIndex(data.model);
        ASSERT(index >= 0);
        context.root_model->RemoveChildModel(index);
        context.name_manager->Remove(data.model->GetName());
    }
    context.selection_manager->ChangeSelection(sel);
}

void ModelExecutorBase::InitModel_(Model &model, CreateModelCommand &command) {
    InitModelTransform_(model, command);
    AddModelInteraction(model);
    SetRandomModelColor(model);

    // If in the main application and the Model was not read from a file, drop
    // it from above.
    if (Util::is_in_main_app && ! command.IsValidating())
        AnimateModelPlacement_(model);
}

void ModelExecutorBase::InitModelTransform_(Model &model,
                                            CreateModelCommand &command) {
    // Make sure the model has an updated mesh, since it is used for computing
    // bounds.
    model.GetMesh();

    // Scale the model by the initial uniform scaling factor.
    if (command.GetInitialScale() != 1)
        model.SetUniformScale(command.GetInitialScale());

    // If the CommandManager is in the middle of validating commands (while
    // loading a session), use the information in the command.
    if (GetContext().command_manager->IsValidating()) {
        model.MoveBottomCenterTo(command.GetTargetPosition(),
                                 command.GetTargetDirection());
    }
    else {
        // This is a new command.
        InitModelPosition(model);

        // Update the command if moving the model to the point target.
        const auto &target_manager = *GetContext().target_manager;
        if (target_manager.IsPointTargetVisible()) {
            const auto &target = target_manager.GetPointTarget();
            command.SetTargetPosition(target.GetPosition());
            command.SetTargetDirection(target.GetDirection());
        }
    }
}

void ModelExecutorBase::AnimateModelPlacement_(Model &model) {
    // Save the current translation as the end point of the animation.
    const Point3f end_pos(model.GetTranslation());

    // Invoke the animation function to place the Model at t=0.
    AnimateModel_(model, end_pos, 0);

    // Start the animation.
    GetContext().animation_manager->StartAnimation(
        [&, end_pos](float t){ return AnimateModel_(model, end_pos, t); });
}

bool ModelExecutorBase::AnimateModel_(Model &model, const Point3f &end_pos,
                                      float time) {
    const Point3f start_pos = end_pos + GetAxis(1, TK::kModelAnimationHeight);
    const float duration = TK::kModelAnimationDuration;
    if (time < duration) {
        // Animation still running.
        model.SetTranslation(
            Vector3f(start_pos + (time / duration) * (end_pos - start_pos)));
        return true;
    }
    else {
        // The animation has completed. Make sure the Model is in the correct
        // spot and select it.
        model.SetTranslation(end_pos);
        GetContext().selection_manager->ReselectAll();
        return false;
    }
}

ModelExecutorBase::ExecData_ & ModelExecutorBase::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ExecData_ *data = new ExecData_;
        data->model = CreateModel(command);
        InitModel_(*data->model, GetTypedCommand<CreateModelCommand>(command));
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
