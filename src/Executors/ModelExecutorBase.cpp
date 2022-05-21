#include "Executors/ModelExecutorBase.h"

#include "App/SelPath.h"
#include "App/Selection.h"
#include "Commands/CreateModelCommand.h"
#include "Managers/AnimationManager.h"
#include "Managers/CommandManager.h"
#include "Managers/NameManager.h"
#include "Managers/SelectionManager.h"
#include "Managers/TargetManager.h"
#include "Targets/PointTarget.h"
#include "Util/General.h"

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

void ModelExecutorBase::InitModelTransform(Model &model, float default_scale,
                                           CreateModelCommand &command) {
    // Make sure the model has an updated mesh, since it is used for computing
    // bounds.
    model.GetMesh();

    // Scale the model by the default uniform scaling factor.
    if (default_scale != 1)
        model.SetUniformScale(default_scale);

    // If the CommandManager is in the middle of validating commands (while
    // loading a session), use the information in the command.
    if (GetContext().command_manager->IsValidating()) {
        model.MoveBottomCenterTo(command.GetTargetPosition(),
                                 command.GetTargetDirection());
    }
    else {
        // This is a new command. Use the point target if it is visible and
        // store the values in the command.
        auto target_manager = GetContext().target_manager;
        if (target_manager->IsPointTargetVisible()) {
            const auto &target = target_manager->GetPointTarget();
            command.SetTargetPosition(target.GetPosition());
            command.SetTargetDirection(target.GetDirection());
            model.MoveBottomCenterTo(target.GetPosition(),
                                     target.GetDirection());
        }
        // If not, use the origin and Y axis and leave the command alone.
        else {
            model.MoveBottomCenterTo(Point3f::Zero(), Vector3f::AxisY());
        }
    }
}

void ModelExecutorBase::AnimateModelPlacement(Model &model) {
    // Don't animate inside unit tests!
    if (Util::is_in_unit_test)
        return;

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
    const Point3f start_pos = end_pos + Vector3f(0, 100, 0);
    const float duration = 1;  // Seconds for the animation.
    if (time < duration) {
        // Animation still running.
        model.SetTranslation(
            Vector3f(start_pos + time * (end_pos - start_pos)));
        return true;
    }
    else {
        // The animation has completed. Make sure the Model is in the correct
        // spot and tell the SelectionManager to reselect to keep the tool
        // placed correctly.
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
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}
