#pragma once

#include "Enums/Hand.h"
#include "Handlers/Handler.h"

#include "SG/Typedefs.h"

class Scene;

//! The Controller class encapsulates everything necessary for displaying and
//! interacting with VR controllers.
//! TODO: Add laser/grip guides and feedback.
class Controller : public Handler {
  public:
    //! The constructor is passed the Hand the controller represents and the
    //! Graph::Node representing the controller in the scene, and the initial
    //! enabled state.
    Controller(Hand hand, const SG::NodePtr &node, bool enabled);
    virtual ~Controller();

    // ------------------------------------------------------------------------
    // Handler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    //! Hand the controller represents.
    const Hand hand_;

    //! Node representing the Controller model.
    SG::NodePtr node_;
};
