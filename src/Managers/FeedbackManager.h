#pragma once

#include <forward_list>
#include <memory>
#include <typeindex>
#include <unordered_map>

#include "Feedback/Feedback.h"
#include "SG/Typedefs.h"
#include "Util/Assert.h"
#include "Util/General.h"

/// The FeedbackManager manages instances of derived Feedback classes so they
/// can be shared and reused by all Tools that need them.
///
/// \ingroup Managers
class FeedbackManager {
  public:
    /// Typedef for function returning scene bounds.
    typedef std::function<Bounds()> SceneBoundsFunc;

    /// Sets the SG::Node instances that are to be used as parents for all
    /// active Feedback objects in world and stage coordinates. It is assumed
    /// that these nodes define the correct (world or stage) coordinate system.
    void SetParentNodes(const SG::NodePtr &world_parent,
                        const SG::NodePtr &stage_parent);

    /// Sets a function that returns the Bounds of all Models in the
    /// scene. This is used to help determine feedback placement.
    void SetSceneBoundsFunc(const SceneBoundsFunc &func);

    /// Stores a template for a specific type of Feedback. The template is used
    /// to create instances of that type.
    template <typename T> void AddTemplate(const std::shared_ptr<T> &temp) {
        std::type_index type_key(typeid(T));
        ASSERT(! Util::MapContains(template_map_, type_key));
        template_map_[type_key] = temp;

        // Create (empty) storage for the available instances.
        InitAvailableList_(type_key);
    }

    /// Clears all templates that have been added. This should be called when
    /// reloading the scene before adding templates again.
    void ClearTemplates();

    /// Activates and returns an instance of the templated type of Feedback.
    /// Asserts if anything goes wrong.
    template <typename T> std::shared_ptr<T> Activate() {
        // Check available instances. If none is available, clone a new one.
        std::type_index type_key(typeid(T));
        std::shared_ptr<T> instance =
            Util::CastToDerived<T>(GetAvailableInstance_(type_key));
        if (! instance)
            instance = template_map_[type_key]->CloneTyped<T>(true);
        ActivateInstance_(Util::CastToBase<Feedback>(instance));
        return instance;
    }

    /// Same as Activate(), but assigns a key to the instance that can be used
    /// to find it later to deactivate it.
    template <typename T>
    std::shared_ptr<T> ActivateWithKey(const std::string &key) {
        std::shared_ptr<T> instance = Activate<T>();
        AddActiveInstance_(key, instance);
        return instance;
    }

    /// Deactivates the given Feedback instance.
    template <typename T> void Deactivate(const std::shared_ptr<T> &instance) {
        // Deactivate the instance and add it to the available list.
        DeactivateInstance_(Util::CastToBase<Feedback>(instance));
        MakeAvailable_(instance, typeid(T));
    }

    /// Same as Deactivate(), but looks up the Feedback instance by key.
    /// Asserts if it is not found.
    template <typename T> void DeactivateWithKey(const std::string &key) {
        auto instance = FindActiveInstance_(key);
        ASSERT(Util::CastToDerived<T>(instance));
        DeactivateInstance_(instance);
        MakeAvailable_(instance, typeid(T));
        RemoveActiveInstance_(key, instance);
    }

  private:
    /// Typedef for available instance storage. Use a forward_list to make
    /// insertion and removal easier - no need for random access.
    typedef std::forward_list<FeedbackPtr> AvailableList_;

    /// Parent Node for active Feedback instances that operate in world
    /// coordinates.
    SG::NodePtr world_parent_node_;

    /// Parent Node for active Feedback instances that operate in stage
    /// coordinates.
    SG::NodePtr stage_parent_node_;

    /// Function to invoke to get the scene bounds.
    SceneBoundsFunc scene_bounds_func_;

    /// This stores the template for each derived Feedback class, keyed by
    /// type_index.
    std::unordered_map<std::type_index, FeedbackPtr> template_map_;

    /// This stores the available instances for each type of Feedback, keyed by
    /// type_index.
    std::unordered_map<std::type_index, AvailableList_> available_instances_;

    /// This stores the active instances for all types of Feedback, keyed by
    /// user-defined string.
    std::unordered_map<std::string, FeedbackPtr> active_instances_;

    /// Initializes a list of available instances for the given type key.
    void InitAvailableList_(const std::type_index &type_key);

    /// Returns an available instance with the given type key, or a null
    /// pointer if none is available.
    FeedbackPtr GetAvailableInstance_(const std::type_index &type_key);

    /// Adds the given instance to the available list with the type key.
    void MakeAvailable_(const FeedbackPtr &instance,
                        const std::type_index &type_key);

    /// Activates an instance and adds it to the appropriate parent.
    void ActivateInstance_(const FeedbackPtr &instance);

    /// Deactivates an instance and removes it from the appropriate parent.
    void DeactivateInstance_(const FeedbackPtr &instance);

    /// Associates an active instance with a key.
    void AddActiveInstance_(const std::string &key,
                            const FeedbackPtr &instance);

    /// Removes the association of an active instance with a key.
    void RemoveActiveInstance_(const std::string &key,
                               const FeedbackPtr &instance);

    /// Returns the active instance with the given key, or a null pointer if
    /// there is none.
    FeedbackPtr FindActiveInstance_(const std::string &key);
};

typedef std::shared_ptr<FeedbackManager> FeedbackManagerPtr;
