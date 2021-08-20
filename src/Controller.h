#pragma once

#include "Hand.h"
#include "Interfaces/IHandler.h"

#include "SG/Typedefs.h"

class Scene;

//! The Controller class encapsulates everthing necessary for displaying and
//! interacting with VR controllers.
//! TODO: Add laser/grip guides and feedback.
class Controller : public IHandler {
  public:
    //! The constructor is passed the Hand the controller represents and the
    //! Graph::Node representing the controller in the scene.
    Controller(Hand hand, const SG::NodePtr &node);
    virtual ~Controller();

    virtual const char * GetClassName() const override { return "Controller"; }

    // ------------------------------------------------------------------------
    // IHandler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    //! Hand the controller represents.
    const Hand hand_;

    //! Node representing the Controller model.
    SG::NodePtr node_;
};
