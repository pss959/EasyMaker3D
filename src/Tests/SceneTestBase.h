#pragma once

#include <concepts>

#include "SG/IonContext.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "Tests/TestBaseWithTypes.h"
#include "Tests/Testing.h"

// This is a base class for any test that has to read a scene from a file or
// string.
class SceneTestBase : public TestBaseWithTypes {
 protected:
    // Default PrimitiveModel base scale. This is a shorthand convenience since
    // this value is used in lots of tests.
    static const float MS;

    // The destructor resets any static state.
    virtual ~SceneTestBase();

    // Builds a string defining a Scene with the given root node contents
    // (between the curly braces).
    std::string BuildSceneString(const std::string &contents);

    // Creates a TempFile containing the given input, tries to read a Scene
    // from it, and returns the Scene after removing the file. If set_up_ion
    // is true (the default), this sets up Ion for the scene.
    SG::ScenePtr ReadScene(const std::string &input, bool set_up_ion = true);

    // Creates an instance of an object derived from SG::Node by parsing it
    // from the given string. If set_up_ion is true, this also reads the Scene
    // from "RealScene.emd" and adds the object as a child to it, saving the
    // Scene from being destroyed. The object is returned.
    template <typename T>
    std::shared_ptr<T> ReadTypedNode(const std::string &input,
                                     bool set_up_ion) {
        static_assert(std::derived_from<T, SG::Node> == true);
        auto node = ParseObject<T>(input);
        EXPECT_NOT_NULL(node);
        if (set_up_ion) {
            scene_ = ReadScene(ReadDataFile("RealScene.emd"), true);
            // This will set up Ion in the Node.
            scene_->GetRootNode()->AddChild(node);
            EXPECT_NOT_NULL(node->GetIonNode().Get());
        }
        return node;
    }

    // Sets up and returns an IonContext for use in initializing Ion objects.
    // This context is also set up if necessary by ReadScene() and persists
    // until the SceneTestBase instance is destroyed.
    SG::IonContextPtr GetIonContext() {
        if (! ion_context_)
            InitIonContext_();
        return ion_context_;
    }

    // Clears the IonContext so it will be recreated, to avoid pollution within
    // tests.
    void ResetContext() { ion_context_.reset(); }

    // Calls SetUpIon() for the given Node.
    void SetUpIonForNode(SG::Node &node) {
        node.SetUpIon(GetIonContext(),
                      std::vector<ion::gfx::ShaderProgramPtr>());
    }

  private:
    SG::ScenePtr      scene_;  // Saves scene from ReadTypedNode().
    SG::IonContextPtr ion_context_;

    // Sets up the IonContext for use in initializing Ion objects.
    void InitIonContext_();

    Parser::ObjectPtr ReadItem_(const std::string &input);
};
