#include "Executors/ModelExecutorBase.h"

#include "Util/General.h"
#include "Selection.h"
#include "SelPath.h"

void ModelExecutorBase::Execute(Command &command, Command::Op operation) {
    ModelExecData_ &data = GetExecData_(command);

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
    // Use the default uniform scaling factor and compute the Model's
    // bounds using the scale.
    model.SetScale(default_scale * Vector3f(1, 1, 1));

    const Bounds bounds = model.GetScaledBounds();

    // Determine if the target is in effect. If so, use it to place the
    // Model. Otherwise, put it at the origin.
#if XXXX
    TargetManager targetManager = GetContext().targetManager;
    if (targetManager.IsPointTargetVisible()) {
        PointTarget pt = targetManager.GetPointTarget();
        model.MoveBottomCenterTo(pt.position, pt.direction);
    }
    else {
        t.localPosition =
            -UT.GetBoundsFaceCenter(bounds, UT.GetFace(Face.Bottom));
    }
#endif
    model.SetTranslation(
        Vector3f(-bounds.GetFaceCenter(Bounds::Face::kBottom)));
}

void ModelExecutorBase::AnimateModelPlacement(Model &model) {
    // Don't animate inside unit tests!
    if (Util::is_in_unit_test)
        return;

    // Start the animation.
    GetContext().animation_manager->StartAnimation(
        std::bind(&ModelExecutorBase::AnimateModel_, this, std::ref(model),
                  Point3f(model.GetTranslation()), std::placeholders::_1));
}

bool ModelExecutorBase::AnimateModel_(Model &model, const Point3f &end_pos,
                                      float time) {
    const Point3f start_pos = end_pos + Vector3f(0, 100, 0);
    const float duration = 1;  // Seconds for the animation.
    model.SetTranslation(Vector3f(start_pos + time * (end_pos - start_pos)));
    if (time < duration)
        return true;

    // The animation has completed. Tell the SelectionManager to reselect to
    // keep the tool placed correctly.
    GetContext().selection_manager->ReselectAll();
    return false;
}

ModelExecutorBase::ModelExecData_ & ModelExecutorBase::GetExecData_(
    Command &command) {
    // Create the ModelExecData_ if not already done.
    if (! command.GetExecData()) {
        ModelExecData_ *data = new ModelExecData_;
        data->model = CreateModel(command);
        command.SetExecData(data);
    }
    return *static_cast<ModelExecData_ *>(command.GetExecData());
}
