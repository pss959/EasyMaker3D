#pragma once

#include <ion/gfxutils/shadermanager.h>
#include <ion/text/fontmanager.h>

#include "SG/IonContext.h"
#include "SG/Scene.h"
#include "SG/Tracker.h"

class FilePath;

/// The SceneLoader class implements scene loading for applications. It manages
/// all context necessary to read the scene and set it up.
class SceneLoader {
  public:
    SceneLoader();

    /// Reads a scene from the given path. Returns a null SG::ScenePtr if
    /// anything fails.
    SG::ScenePtr LoadScene(const FilePath &path);

    const ion::gfxutils::ShaderManagerPtr & GetShaderManager() {
        return shader_manager_;
    }

    /// Returns the scene being read. This can be called before the scene
    /// loading is complete. If called afterwards, it returns the last scene
    /// read.
    const SG::ScenePtr GetScene() const { return scene_; }

  private:
    SG::TrackerPtr                  tracker_;
    ion::gfxutils::ShaderManagerPtr shader_manager_;
    ion::text::FontManagerPtr       font_manager_;
    SG::ScenePtr                    scene_;
    SG::IonContextPtr               ion_context_;
};
