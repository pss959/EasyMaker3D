#pragma once

#include <ion/gfxutils/shadermanager.h>
#include <ion/text/fontmanager.h>

#include "IO/Reader.h"
#include "SG/IonSetup.h"
#include "SG/Scene.h"
#include "SG/Tracker.h"
#include "RegisterTypes.h"
#include "Testing.h"

// This is a base class for any test that has to read a scene from a file or
// string.
class SceneTestBase : public TestBase {
 protected:
    // Tracker for tracking resources.
    SG::TrackerPtr tracker;

    // ShaderManager for creating shaders.
    ion::gfxutils::ShaderManagerPtr shader_manager;

    // FontManager for text.
    ion::text::FontManagerPtr font_manager;

    // Handy Reader instance.
    Reader reader;

    // Flag indicating whether to set up Ion stuff. (Turning this off is useful
    // for parsing tests that don't care about Ion objects.)
    bool set_up_ion = true;

    // These strings help define a valid scene. Put any node-specific fields
    // between them.
    const std::string str1 =
        "Scene { render_passes: [ LightingPass { shader_programs: ["
        "  <\"programs/BaseColor.mvn\">, ],"
        "root_node: ShaderNode \"Root\" {";
    const std::string str2 = "}}]}";

    SceneTestBase() {
        RegisterTypes();
    }

    ~SceneTestBase() {
        // Clear the registry so the next test has a fresh start.
        UnregisterTypes();
    }

    // Creates a TempFile containing the given input, tries to read a Scene
    // from it, and returns the Scene after removing the file.
    SG::ScenePtr ReadScene(const std::string &input) {
        TempFile file(input);

        // Create new instances to avoid inter-test pollution.
        tracker.reset(new SG::Tracker());
        shader_manager.Reset(new ion::gfxutils::ShaderManager);
        font_manager.Reset(new ion::text::FontManager);

        SG::ScenePtr scene = reader.ReadScene(file.GetPathString(), *tracker);
        if (scene && set_up_ion) {
            SG::IonSetup(*tracker, shader_manager,
                         *font_manager).SetUpScene(*scene);
        }
        return scene;
    }
};
