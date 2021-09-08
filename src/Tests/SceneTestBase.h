#pragma once

#include <ion/gfxutils/shadermanager.h>
#include <ion/text/fontmanager.h>

#include "Reader.h"
#include "SG/Context.h"
#include "SG/Scene.h"
#include "SG/Tracker.h"
#include "Testing.h"

// This is a base class for any test that has to read a scene from a file or
// string.
class SceneTestBase : public TestBase {
 protected:
    SceneTestBase() :
        sg_context(
            new SG::Context(
                SG::TrackerPtr(new SG::Tracker()),
                ion::gfxutils::ShaderManagerPtr(
                    new ion::gfxutils::ShaderManager),
                ion::text::FontManagerPtr(new ion::text::FontManager))) {}

    // SG::Context used for reading and setting up Ion.
    SG::ContextPtr sg_context;

    // Handy Reader instance.
    Reader reader;

    // Flag indicating whether to set up Ion stuff. (Turning this off is useful
    // for parsing tests that don't care about Ion objects.)
    bool set_up_ion = true;

    // Creates a TempFile containing the given input, tries to read a Scene
    // from it, and returns the Scene after removing the file.
    SG::ScenePtr ReadScene(const std::string &input) {
        TempFile file(input);
        SG::ScenePtr scene = reader.ReadScene(file.GetPathString(),
                                              *sg_context->tracker);
        if (scene && set_up_ion)
            scene->SetUpIon(sg_context);
        return scene;
    }
};
