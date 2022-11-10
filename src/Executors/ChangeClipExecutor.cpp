#include "Executors/ChangeClipExecutor.h"

#include <ion/math/transformutils.h>

#include "Commands/ChangeClipCommand.h"
#include "Managers/SelectionManager.h"
#include "Math/Linear.h"
#include "Models/ClippedModel.h"

void ChangeClipExecutor::Execute(Command &command, Command::Op operation) {
    ExecData_ &data = GetExecData_(command);

    for (auto &pm: data.per_model) {
        ClippedModel &cm = GetTypedModel<ClippedModel>(pm.path_to_model);
        if (operation == Command::Op::kDo)
            cm.AddPlane(pm.object_plane);
        else   // Undo.
            cm.RemoveLastPlane();
        AdjustTranslation_(pm);
    }

    // Reselect if undo or if command is finished being done.
    if (operation == Command::Op::kUndo || command.IsFinalized())
        GetContext().selection_manager->ReselectAll();
}

ChangeClipExecutor::ExecData_ & ChangeClipExecutor::GetExecData_(
    Command &command) {
    // Create the ExecData_ if not already done.
    if (! command.GetExecData()) {
        ChangeClipCommand &ccc = GetTypedCommand<ChangeClipCommand>(command);

        const auto &model_names = ccc.GetModelNames();
        ASSERT(! model_names.empty());

        ExecData_ *data = new ExecData_;
        data->per_model.resize(model_names.size());

        for (size_t i = 0; i < model_names.size(); ++i) {
            ExecData_::PerModel &pm = data->per_model[i];
            const SelPath path = FindPathToModel(model_names[i]);
            pm.path_to_model = path;

            // Convert plane from stage to object coordinates.
            const Matrix4f som = CoordConv(path).GetRootToObjectMatrix();
            pm.object_plane = TransformPlane(ccc.GetPlane(), som);
        }
        command.SetExecData(data);
    }
    return *static_cast<ExecData_ *>(command.GetExecData());
}

#include "Math/ToString.h"  // XXXX

void ChangeClipExecutor::AdjustTranslation_(ExecData_::PerModel &pm) {
    ClippedModel &cm   = GetTypedModel<ClippedModel>(pm.path_to_model);
    const auto   &orig = *cm.GetOriginalModel();

#if 1 // XXXX
    // The original mesh and the clipped mesh are both centered on the origin
    // in object coordinates (like all meshes). However, they are different
    // points in local coordinates due to the mesh size change. The
    // ClippedModel stores the offset for this purpose.
    const Matrix4f &mm = cm.GetModelMatrix();
    const Vector3f offset = mm * cm.GetMeshOffset();

#if XXXX
    const Point3f p0 = mm * Point3f::Zero();
    const Point3f p1 = mm * (Point3f::Zero() - cm.GetMeshOffset());

    const Vector3f vx0 = cm.GetScale() * cm.GetMeshOffset();
    const Vector3f vx1 = cm.GetRotation() * vx0;

    std::cerr << "XXXX ======== For " << cm.GetName() << ":\n";
    std::cerr << "XXXX   MM:\n"  << Math::ToString(mm, .01f) << "\n";
    std::cerr << "XXXX   UBD  = " << orig.GetBounds().ToString() << "\n";
    std::cerr << "XXXX   CBD  = " << cm.GetBounds().ToString() << "\n";
    std::cerr << "XXXX   MOFF = " << cm.GetMeshOffset() << "\n";
    std::cerr << "XXXX   TOFF = " << offset << "\n";
    std::cerr << "XXXX   P0   = " << p0 << "\n";
    std::cerr << "XXXX   P1   = " << p1 << "\n";
    std::cerr << "XXXX   DIFF = " << (p1 - p0) << "\n";
    std::cerr << "XXXX   VX0  = " << vx0 << "\n";
    std::cerr << "XXXX   VX1  = " << vx1 << "\n";
    std::cerr << "XXXX   OTR  = " << orig.GetTranslation() << "\n";
    std::cerr << "XXXX   NTR  = " << (orig.GetTranslation() - offset) << "\n";
#endif

    cm.SetTranslation(orig.GetTranslation() - offset);
#endif

#if 0  // XXXX
    // The original mesh and the clipped mesh are both centered on the origin
    // in object coordinates (like all meshes). However, they are different
    // points in local coordinates due to the mesh size change. Compute the
    // amount to translate (in local coordinates) to compensate for this
    // change.
    //
    // The easiest way to do this is to make sure one of the unclipped points
    // of the original model ends up in the same place in local coordinates.
    // Take the corner of the object bounds that has the most negative distance
    // from the plane (also in object coordinates) and see how much it moves in
    // local coordinates, then translate by the opposite.

    const Bounds unclipped_bounds = orig.GetBounds();
    const Bounds   clipped_bounds = cm.GetBounds();
    Point3f unclipped_corners[8];
    Point3f   clipped_corners[8];
    unclipped_bounds.GetCorners(unclipped_corners);
    clipped_bounds.GetCorners(clipped_corners);

    int   stable_corner_index = -1;
    float most_neg_distance = 1;
    for (int i = 0; i < 8; ++i) {
        const float signed_distance =
            pm.object_plane.GetDistanceToPoint(unclipped_corners[i]);
        if (signed_distance < most_neg_distance) {
            stable_corner_index = i;
            most_neg_distance = signed_distance;
        }
    }
    ASSERT(stable_corner_index >= 0);

    // Convert the corner for both the original and clipped bounds to local
    // coordinates and get the difference.
    const Matrix4f &mm = cm.GetModelMatrix();
    const Vector3f offset =
        (mm * unclipped_corners[stable_corner_index]) -
        (mm *   clipped_corners[stable_corner_index]);

    // The original Model stores the translation with no offset. Add the offset
    // to it to get the new translation.
    cm.SetOffset(offset);
    cm.SetTranslation(orig.GetTranslation() + offset);

    std::cerr << "XXXX ======== For " << cm.GetName() << ":\n";
    std::cerr << "XXXX   MM:\n"  << Math::ToString(mm, .01f) << "\n";
    std::cerr << "XXXX   UBD = " << unclipped_bounds.ToString() << "\n";
    std::cerr << "XXXX   CBD = " << clipped_bounds.ToString() << "\n";
    std::cerr << "XXXX   STI = " << stable_corner_index << "\n";
    std::cerr << "XXXX   UC  = " << unclipped_corners[stable_corner_index]
              << "\n";
    std::cerr << "XXXX   CC  = " << clipped_corners[stable_corner_index] << "\n";
    std::cerr << "XXXX   MUC = " << (mm * unclipped_corners[stable_corner_index])
              << "\n";
    std::cerr << "XXXX   MCC = " << (mm * clipped_corners[stable_corner_index])
              << "\n";
    std::cerr << "XXXX   OFF = " << offset << "\n";
    std::cerr << "XXXX   OTR = " << orig.GetTranslation() << "\n";
    std::cerr << "XXXX   NTR = " << (orig.GetTranslation() + offset) << "\n";
#endif
}
