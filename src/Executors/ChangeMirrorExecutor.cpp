//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Executors/ChangeMirrorExecutor.h"

#include "Math/Linear.h"
#include "Models/MirroredModel.h"
#include "Util/Assert.h"

Plane ChangeMirrorExecutor::GetModelPlane(const Model &model) const {
    ASSERT(dynamic_cast<const MirroredModel *>(&model));
    return Plane(0, static_cast<const MirroredModel &>(model).GetPlaneNormal());
}

void ChangeMirrorExecutor::UpdateModel(Model &model,
                                       const PlaneData &data) const {
    ASSERT(dynamic_cast<MirroredModel *>(&model));
    auto &mm = static_cast<MirroredModel &>(model);

    // Convert the new plane from stage coordinates into object coordinates.
    const Plane new_object_plane =
        TransformPlane(data.new_stage_plane, data.cc.GetRootToObjectMatrix());

    // Set the plane normal in the MirroredModel.
    mm.SetPlaneNormal(new_object_plane.normal);

    // Update the translation for the MirroredModel. If the center of the
    // operand Model mirrored across the old Plane is not the same as (or close
    // to) the old translation of the MirroredModel, this means that the
    // MirroredModel was translated after conversion. Compute the difference
    // between the two and add that back in to the new translation.
    const auto obj_center = Point3f(mm.GetOperandModel()->GetTranslation());
    const auto diff = Point3f(data.old_translation) -
        data.old_object_plane.MirrorPoint(obj_center);
    const auto stage_center   = data.cc.LocalToRoot(obj_center);
    const auto stage_mirrored = data.new_stage_plane.MirrorPoint(stage_center);
    mm.TranslateTo(data.cc.RootToLocal(stage_mirrored) + diff);
}

void ChangeMirrorExecutor::SetModelPlane(Model &model,
                                         const Plane &plane) const {
    ASSERT(dynamic_cast<MirroredModel *>(&model));
    auto &mm = static_cast<MirroredModel &>(model);
    mm.SetPlaneNormal(plane.normal);
}
