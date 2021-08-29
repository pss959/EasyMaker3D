#pragma once

#include <memory>

#include "Event.h"
#include "Interfaces/IHandler.h"
#include "SceneContext.h"
#include "SG/Scene.h"

//! MainHandler is a derived IHandler that does most of the interactive event
//! handling for the application.
class MainHandler : public IHandler {
  public:
    MainHandler();
    virtual ~MainHandler();

    //! Sets the SceneContext to interact with.
    void SetSceneContext(std::shared_ptr<SceneContext> context) {
        context_ = context;
    }

    virtual const char * GetClassName() const override { return "MainHandler"; }

    // ------------------------------------------------------------------------
    // IHandler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;

  private:
    std::shared_ptr<SceneContext> context_;  //!< SceneContext interacting with.
};
