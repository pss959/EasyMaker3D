#pragma once

#include <ion/gfxutils/shadermanager.h>
#include <ion/text/fontmanager.h>

#include "Reader.h"
#include "SG/IonSetup.h"
#include "SG/Scene.h"
#include "SG/Tracker.h"
#include "Testing.h"

// This is a base class for any test that has to read a scene from a file or
// string.
class SceneTestBase : public TestBase {
 protected:
    // Tracker for tracking resources.
    SG::TrackerPtr tracker{new SG::Tracker()};

    // ShaderManager for creating shaders.
    ion::gfxutils::ShaderManagerPtr shader_manager{
        new ion::gfxutils::ShaderManager};

    // FontManager for text.
    ion::text::FontManagerPtr font_manager{new ion::text::FontManager};

    // Handy Reader instance.
    Reader reader;

    // Flag indicating whether to set up Ion stuff. (Turning this off is useful
    // for parsing tests that don't care about Ion objects.)
    bool set_up_ion = true;

    // Creates a TempFile containing the given input, tries to read a Scene
    // from it, and returns the Scene after removing the file.
    SG::ScenePtr ReadScene(const std::string &input) {
        TempFile file(input);
        SG::ScenePtr scene = reader.ReadScene(file.GetPathString(), *tracker);
        if (scene && set_up_ion)
            SG::IonSetup(*tracker, shader_manager,
                         *font_manager).SetUpScene(*scene);
        return scene;
    }
};
