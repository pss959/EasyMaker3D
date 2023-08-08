#pragma once

#include <concepts>
#include <string>
#include <vector>

#include "SG/IonContext.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "Tests/TestBaseWithTypes.h"

/// This is a base class for any test that has to read a scene from a file or
/// string and possibly set up Ion data structures for the resulting scene.
/// \ingroup Tests
class SceneTestBase : public TestBaseWithTypes {
 protected:
    /// Default PrimitiveModel base scale. This is a shorthand convenience
    /// since this value is used in lots of tests.
    static const float MS;

    /// The destructor resets any static state.
    virtual ~SceneTestBase();

    /// Creates a temporary file containing the given input, tries to read a
    /// Scene from it, and returns the Scene. If set_up_ion is true (the
    /// default), this also calls SetUpIon() for the scene.
    SG::ScenePtr ReadScene(const std::string &input, bool set_up_ion = true);

    /// Same as ReadScene(), except it takes just the contents of the Scene's
    /// root node (between the curly braces), building the rest of the Scene
    /// around it first.
    SG::ScenePtr BuildAndReadScene(const std::string &contents,
                                   bool set_up_ion = true);


    /// Calls ReadScene() for "RealScene.emd" after inserting include
    /// statements for each of the given files as children of the root node.
    SG::ScenePtr ReadSceneWithIncludes(
        const std::vector<std::string> &inc_file_names);

    /// Parses an instance of an object derived from SG::Node from the given
    /// string, adds it as a child of the "RealScene.emd" Scene, and calls
    /// SetUpIon() for the Scene. Returns the instance.
    template <typename T>
    std::shared_ptr<T> ParseAndSetUpNode(const std::string &input) {
        static_assert(std::derived_from<T, SG::Node> == true);
        auto node = ParseObject<T>(input);
        AddNodeToRealScene_(node);
        return node;
    }

    /// Similar to ParseAndSetUpNode(), but uses an include statement for the
    /// named file as the contents of the scene, then searches for and returns
    /// the named instance of the templated type.
    template <typename T>
    std::shared_ptr<T> ReadAndSetUpNode(const std::string &file_name,
                                        const std::string &instance_name) {
        static_assert(std::derived_from<T, SG::Node> == true);
        scene_ = ReadSceneWithIncludes(std::vector<std::string>(1, file_name));
        return SG::FindTypedNodeInScene<T>(*scene_, instance_name);
    }

    /// Returns the Scene read in by any of the above scene-reading functions.
    /// This will be null if none was called successfully.
    SG::ScenePtr GetScene() const { return scene_; }

    /// Sets up and returns an IonContext for use in initializing Ion objects.
    /// This context is also set up if necessary by ReadScene() and persists
    /// until the SceneTestBase instance is destroyed.
    SG::IonContextPtr GetIonContext();

    /// Clears the IonContext so it will be recreated, to avoid pollution
    /// within tests.
    void ResetContext() { ion_context_.reset(); }

  private:
    SG::ScenePtr      scene_;        ///< Saves scene from ParseAndSetUpNode().
    SG::IonContextPtr ion_context_;  ///< IonContext used for SetUpIon().

    /// Sets up the IonContext for use in initializing Ion objects.
    void InitIonContext_();

    /// Does most of the work for ParseAndSetUpNode().
    void AddNodeToRealScene_(const SG::NodePtr &node);

    /// Does most of the work for ReadAndSetUpNode().
    void ReadSceneWithInclude_(const std::string &file_name);
};
