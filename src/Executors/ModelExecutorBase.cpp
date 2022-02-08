#include "Executors/ModelExecutorBase.h"

#include "Util/General.h"
#include "Selection.h"
#include "SelPath.h"

void ModelExecutorBase::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    // Process the command.
    const Context &context = GetContext();
    Selection sel;
    if (operation == Command::Op::kDo) {
        // Add as a top-level Model and select it.
        context.root_model->AddChildModel(data.model);
        sel.Add(SelPath(context.root_model, data.model));
    }
    else {  // Undo.
        const int index = context.root_model->GetChildModelIndex(data.model);
        ASSERT(index >= 0);
        context.root_model->RemoveChildModel(index);
    }
    context.selection_manager->ChangeSelection(sel);
}

void ModelExecutorBase::InitModelTransform(Model &model, float default_scale) {
    // Make sure the model has an updated mesh, since it is used for computing
    // bounds.
    model.GetMesh();

    // Scale the model by the default uniform scaling factor.
    model.SetUniformScale(default_scale);

    // Determine if the target is in effect. If so, use it to place the
    // Model. Otherwise, put it at the origin.
    auto target_manager = GetContext().target_manager;
    if (target_manager->IsPointTargetVisible()) {
        const auto &target = target_manager->GetPointTarget();
        model.MoveBottomCenterTo(target.GetPosition(), target.GetDirection());
    }
    else {
        model.MoveBottomCenterTo(Point3f::Zero(), Vector3f::AxisY());
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
        model.SetTranslation(Vector3f(end_pos));
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
