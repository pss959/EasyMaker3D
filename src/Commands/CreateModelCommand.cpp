//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/CreateModelCommand.h"

#include "Models/Model.h"
#include "Util/Assert.h"

void CreateModelCommand::AddFields() {
    AddField(result_name_.Init("result_name"));

    AddField(initial_scale_.Init("initial_scale", 1));
    AddField(target_position_.Init("target_position", Point3f::Zero()));
    AddField(target_direction_.Init("target_direction", Vector3f::AxisY()));

    Command::AddFields();
}

bool CreateModelCommand::IsValid(Str &details) {
    if (! Command::IsValid(details))
        return false;  // LCOV_EXCL_LINE [cannot happen]
    if (! Model::IsValidName(result_name_)) {
        details = "Invalid model name";
        return false;
    }
    return true;
}

void CreateModelCommand::SetResultName(const Str &name) {
    ASSERT(Model::IsValidName(name));
    result_name_ = name;
}

Str CreateModelCommand::BuildDescription(
    const Str &model_type) const {
    return "Created " + model_type + " Model \"" + GetResultName() + "\"";
}
