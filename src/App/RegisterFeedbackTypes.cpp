//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Feedback/AngularFeedback.h"
#include "Feedback/LinearFeedback.h"
#include "Feedback/TooltipFeedback.h"
#include "Parser/Registry.h"

#define REGISTER_TYPE_(T) Parser::Registry::AddType<T>(#T)

void RegisterFeedbackTypes_();

void RegisterFeedbackTypes_() {
    REGISTER_TYPE_(AngularFeedback);
    REGISTER_TYPE_(LinearFeedback);
    REGISTER_TYPE_(TooltipFeedback);
}

#undef REGISTER_TYPE_

