#pragma once

#include <ion/gfxutils/shadermanager.h>
#include <ion/text/fontmanager.h>

namespace SG {

class Scene;
class Tracker;

//! The IonSetup class sets up all of the Ion data in an SG graph.
class IonSetup {
  public:
    //! Sets up the Ion data in the given Scene. All of the necessary context
    //! is passed in.
    static void SetUpScene(Scene &scene, Tracker &tracker,
                           ion::gfxutils::ShaderManager &shader_manager,
                           ion::text::FontManager &font_manager);
};

}  // namespace SG
