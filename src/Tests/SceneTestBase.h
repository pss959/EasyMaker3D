#pragma once

#include <ion/gfxutils/shadermanager.h>
#include <ion/text/fontmanager.h>

#include "Reader.h"
#include "SG/Scene.h"
#include "SG/Tracker.h"
#include "Testing.h"

// This is a base class for any test that has to read a scene from a file or
// string.
class SceneTestBase : public TestBase {
 protected:
    SceneTestBase() :
        shader_manager(new ion::gfxutils::ShaderManager),
        font_manager(new ion::text::FontManager),
        reader(tracker, shader_manager, font_manager) {}

    // ShaderManager used to create shaders.
    ion::gfxutils::ShaderManagerPtr shader_manager;

    // FontManager used for text.
    ion::text::FontManagerPtr font_manager;

    // Tracker used for resources.
    SG::Tracker tracker;

    // Handy SG::Reader instance.
    SG::Reader reader;

    // Flag indicating whether to set up Ion stuff. (Turning this off is useful
    // for parsing tests that don't care about Ion objects.)
    bool set_up_ion = true;

    // Creates a TempFile containing the given input, tries to read a Scene
    // from it, and returns the Scene after removing the file.
    SG::ScenePtr ReadScene(const std::string &input) {
        TempFile file(input);
        return reader.ReadScene(file.GetPathString(), set_up_ion);
    }
};
