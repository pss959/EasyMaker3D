#pragma once

#include "Hand.h"
#include "Interfaces/IHandler.h"
#include "Graph/Typedefs.h"

class Scene;

//! The Controller class encapsulates everthing necessary for displaying and
//! interacting with VR controllers.
// XXXX More....
class Controller : public IHandler {
  public:
    //! The constructor is passed the Hand the controller represents and the
    //! Graph::Node representing the controller in the scene.
    Controller(Hand hand, const Graph::NodePtr &node);
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

    //! Node representing the Controller model.
    Graph::NodePtr node_;
};
