#include "Feedback/FindFeedback.h"

#include "SG/Search.h"

std::vector<FeedbackPtr> FindFeedback(const SG::Node &root_node) {
    std::vector<FeedbackPtr> feedback;

    auto add_feedback = [&](const std::string &type_name){
        feedback.push_back(SG::FindTypedNodeUnderNode<Feedback>(root_node,
                                                                type_name));
    };

    add_feedback("AngularFeedback");
    add_feedback("LinearFeedback");
    add_feedback("TooltipFeedback");

    return feedback;
}
