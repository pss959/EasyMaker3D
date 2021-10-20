#pragma once

#include <memory>

#include "Enums/Hand.h"
#include "Handlers/Handler.h"

#include "SG/Typedefs.h"

/// The Controller class encapsulates everything necessary for displaying and
/// interacting with VR controllers.
/// TODO: Add laser/grip guides and feedback.
class Controller : public Handler {
  public:
    /// The constructor is passed the Hand the controller represents.
    explicit Controller(Hand hand);
    virtual ~Controller();

    /// Sets the Node representing the controller in the scene and its initial
    /// enabled state
    void SetNode(const SG::NodePtr &node, bool enabled);

    // ------------------------------------------------------------------------
    // Handler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    /// Hand the controller represents.
    const Hand hand_;

    /// Node representing the Controller model.
    SG::NodePtr node_;
};

typedef std::shared_ptr<Controller> ControllerPtr;
