#pragma once

#include <ion/gfxutils/shadermanager.h>
#include <ion/text/fontmanager.h>

#include "IO/Reader.h"
#include "SG/IonContext.h"
#include "SG/Scene.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

// This is a base class for any test that has to read a scene from a file or
// string.
class SceneTestBase : public TestBaseWithTypes {
 protected:
    // Default PrimitiveModel base scale. This is a shorthand convenience since
    // this value is used in lots of tests.
    static const float MS;

    // ShaderManager for creating shaders.
    ion::gfxutils::ShaderManagerPtr shader_manager;

    // FontManager for text.
    ion::text::FontManagerPtr font_manager;

    // Handy Reader instance.
    Reader reader;

    // Flag indicating whether to set up Ion stuff. (Turning this off is useful
    // for parsing tests that don't care about Ion objects.)
    bool set_up_ion = true;

    // IonContext for setting up Ion stuff when needed.
    SG::IonContextPtr ion_context;

    // These strings help define a valid scene. Put any node-specific fields
    // between them.
    static const std::string str1;
    static const std::string str2;

    // Creates a TempFile containing the given input, tries to read a Scene
    // from it, and returns the Scene after removing the file. If #set_up_ion
    // is true, this sets up Ion for the scene.
    SG::ScenePtr ReadScene(const std::string &input);

    // Creates a TempFile containing the given input, tries to read an SG item
    // of the templated type from it, and returns the item after removing the
    // file. Returns a null pointer on failure. Note that this does not set up
    // Ion for any items.
    template <typename T>
    std::shared_ptr<T> ReadTypedItem(const std::string &input) {
        return std::dynamic_pointer_cast<T>(ReadItem_(input));
    }

  private:
    Parser::ObjectPtr ReadItem_(const std::string &input);
};
