#include "SG/Visitor.h"

#include "Assert.h"
#include "SG/Node.h"

namespace SG {

Visitor::TraversalCode Visitor::Visit(const NodePtr &root) {
    cur_path_.clear();
    return Visit_(root);
}

Visitor::TraversalCode Visitor::Visit_(const NodePtr &root) {
    if (! root)
        return TraversalCode::kContinue;

    // Visit the root.
    cur_path_.push_back(root);
    TraversalCode code = VisitNodeStart(cur_path_);

    // Do not traverse children if kPrune or kStop.
    if (code == TraversalCode::kContinue) {
        for (const auto &child: root->GetChildren()) {
            code = Visit_(child);

            // Stop visiting children if kStop is returned.
            if (code == TraversalCode::kStop)
                break;
        }
    }

    VisitNodeEnd(cur_path_);

    ASSERT(cur_path_.back() == root);
    cur_path_.pop_back();
    return code;
}

}  // namespace SG
