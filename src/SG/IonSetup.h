#pragma once

#include <memory>

#include <ion/gfxutils/shadermanager.h>
#include <ion/text/fontmanager.h>

namespace SG {

class Scene;
class Tracker;

/// The IonSetup class sets up all of the Ion data in an SG graph.
class IonSetup {
  public:
    /// The constructor is given all of the necessary context.
    IonSetup(Tracker &tracker,
             const ion::gfxutils::ShaderManagerPtr &shader_manager,
             ion::text::FontManager &font_manager);

    /// Sets up the Ion data in the given Scene.
    void SetUpScene(Scene &scene);

  private:
    class Impl_;

    std::shared_ptr<Impl_>       impl_;
};

}  // namespace SG
