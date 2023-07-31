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

    // Builds a string defining a Scene with the given root node contents
    // (between the curly braces).
    std::string BuildSceneString(const std::string &contents);

    // Creates a TempFile containing the given input, tries to read a Scene
    // from it, and returns the Scene after removing the file. If set_up_ion
    // is true (the default), this sets up Ion for the scene.
    SG::ScenePtr ReadScene(const std::string &input, bool set_up_ion = true);

    // Creates a TempFile containing the given input, tries to read an SG item
    // of the templated type from it, and returns the item after removing the
    // file. Returns a null pointer on failure. Note that this does not set up
    // Ion for any items.
    template <typename T>
    std::shared_ptr<T> ReadTypedItem(const std::string &input) {
        return std::dynamic_pointer_cast<T>(ReadItem_(input));
    }

    // Sets up and returns an IonContext for use in initializing Ion objects.
    // This context is also set up if necessary by ReadScene() and persists
    // until the SceneTestBase instance is destroyed.
    SG::IonContextPtr GetIonContext() {
        if (! ion_context_)
            InitIonContext_();
        return ion_context_;
    }

  private:
    SG::IonContextPtr ion_context_;

    // Sets up the IonContext for use in initializing Ion objects.
    void InitIonContext_();

    Parser::ObjectPtr ReadItem_(const std::string &input);
};
