#pragma once

#include "Hand.h"
#include "Interfaces/IHandler.h"

//! The Controller class encapsulates everthing necessary for displaying and
//! interacting with VR controllers.
// XXXX More....
class Controller : public IHandler {
  public:
    //! The constructor is passed the Hand the controller represents.
    Controller(Hand hand);
    virtual ~Controller();

    virtual const char * GetClassName() const override { return "Controller"; }

    // ------------------------------------------------------------------------
    // IHandler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    //! Hand the controller represents.
    const Hand hand_;
};
