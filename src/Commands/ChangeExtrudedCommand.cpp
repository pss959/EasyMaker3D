//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Commands/ChangeExtrudedCommand.h"

#include "Models/ExtrudedModel.h"
#include "Parser/Registry.h"
#include "Util/Assert.h"

void ChangeExtrudedCommand::AddFields() {
    AddField(profile_points_.Init("profile_points"));

    MultiModelCommand::AddFields();
}

bool ChangeExtrudedCommand::IsValid(Str &details) {
    if (! MultiModelCommand::IsValid(details))
        return false;
    if (! GetProfile().IsValid()) {
        details = "Invalid profile";
        return false;
    }
    return true;
}

Str ChangeExtrudedCommand::GetDescription() const {
    return "Changed the profile in " + GetModelsDesc(GetModelNames());
}

void ChangeExtrudedCommand::SetProfile(const Profile &profile) {
    // Make sure the profile is set up for a ExtrudedModel.
    ASSERT(profile.GetType() == Profile::Type::kClosed);
    ASSERT(profile.GetMinPointCount() == 3U);
    ASSERT(profile.IsValid());

    profile_points_ = profile.GetMovablePoints();
}

Profile ChangeExtrudedCommand::GetProfile() const {
    return ExtrudedModel::CreateProfile(profile_points_);
}
