#pragma once

#include <ion/gfx/node.h>

#include "Hand.h"
#include "Interfaces/IHandler.h"

class Scene;

//! The Controller class encapsulates everthing necessary for displaying and
//! interacting with VR controllers.
// XXXX More....
class Controller : public IHandler {
  public:
    //! The constructor is passed the Hand the controller represents.
    Controller(Hand hand);
    virtual ~Controller();

    virtual const char * GetClassName() const override { return "Controller"; }

    //! Adds a model representing the Controller to the given Scene.
    void AddModelToScene(Scene &scene);

    // ------------------------------------------------------------------------
    // IHandler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    //! Hand the controller represents.
    const Hand hand_;

    //! Root node of the Controller model.
    ion::gfx::NodePtr node_;

    //! Index of the uModelviewMatrix uniform in the node.
    int matrix_index_;

    //! XXXX
    void BuildShape();
};
