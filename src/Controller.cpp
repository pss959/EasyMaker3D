#include "Controller.h"

#include "Assert.h"
#include "Event.h"
#include "Math/Types.h"
#include "SG/Node.h"

// ----------------------------------------------------------------------------
// Controller implementation.
// ----------------------------------------------------------------------------

Controller::Controller(Hand hand) : hand_(hand) {
}

Controller::~Controller() {
}

void Controller::SetNode(const SG::NodePtr &node, bool enabled) {
    node_ = node;
    node_->SetEnabled(SG::Node::Flag::kTraversal, enabled);
}

bool Controller::HandleEvent(const Event &event) {
    // Track the corresponding controller.
    Event::Device this_device = hand_ == Hand::kLeft ?
        Event::Device::kLeftController : Event::Device::kRightController;
    if (event.device == this_device &&
        event.flags.Has(Event::Flag::kPosition3D) &&
        event.flags.Has(Event::Flag::kOrientation)) {
        if (event.orientation.IsIdentity()) {
            // If the orientation is identity, the controller is not active, so
            // hide the model.
            node_->SetEnabled(SG::Node::Flag::kTraversal, false);
        }
        else {
            node_->SetEnabled(SG::Node::Flag::kTraversal, true);
            node_->SetTranslation(Vector3f(event.position3D));
            node_->SetRotation(event.orientation);
        }
    }
    // No need to trap these events - others may be interested.
    return false;
}
