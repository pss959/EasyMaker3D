#pragma once

#include <memory>

#include "Interfaces/IApplication.h"

//! Application is an implementation of the IApplication interface. It is
//! basically a factory that produces instances of implementations of all
//! required interfaces.
class Application : public IApplication {
  public:
    virtual IRenderer & GetRenderer() override;
    virtual IScene &    GetScene() override;
    virtual IVR &       GetVR() override;

  private:
    std::unique_ptr<IRenderer> renderer_;
    std::unique_ptr<IScene>    scene_;
    std::unique_ptr<IVR>       vr_;
};
