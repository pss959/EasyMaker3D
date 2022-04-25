#pragma once

#include <vector>

#include "SG/Node.h"
#include "Feedback/Feedback.h"

/// Finds all Feedback instances under the given root Node and returns a vector
/// containing them.
///
/// \ingroup Feedback
std::vector<FeedbackPtr> FindFeedback(const SG::Node &root_node);
