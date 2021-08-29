#include "SG/Visitor.h"

#include "SG/Node.h"

namespace SG {

Visitor::TraversalCode Visitor::Visit(const NodePtr &root, TraversalFunc func) {
    if (! root)
        return TraversalCode::kContinue;

    // Visit the root.
    cur_path_.clear();
    cur_path_.push_back(root);
    TraversalCode code = func(cur_path_);

    // Do not traverse children if kPrune or kStop.
    if (code == TraversalCode::kContinue) {
        for (const auto &child: root->GetChildren()) {
            code = Visit(child, func);

            // Stop visiting children if kStop is returned.
            if (code == TraversalCode::kStop)
                break;
        }
    }
    return code;
}

}  // namespace SG
