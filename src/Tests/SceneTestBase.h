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


    /// Calls ReadScene() for "RealScene.emd" after inserting the given
    /// contents within the root node. Calls SetUpIon() on the results.
    SG::ScenePtr ReadRealScene(const std::string &contents);

    /// Calls ReadRealScene() with the given contents, then searches for and
    /// returns the named Node of the templated type.
    template <typename T>
    std::shared_ptr<T> ReadRealNode(const std::string &contents,
                                    const std::string &node_name) {
        static_assert(std::derived_from<T, SG::Node> == true);
        scene_ = ReadRealScene(contents);
        return SG::FindTypedNodeInScene<T>(*scene_, node_name);
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

    /// Uses the IonContext to call SetUpIon() for a Node.
    void SetUpIonForNode(SG::Node &node);

  private:
    SG::ScenePtr      scene_;        ///< Saves scene from ReadScene().
    SG::IonContextPtr ion_context_;  ///< IonContext used for SetUpIon().

    /// Sets up the IonContext for use in initializing Ion objects.
    void InitIonContext_();
};
