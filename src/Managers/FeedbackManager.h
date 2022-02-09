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
        std::type_index key(typeid(T));
        ASSERT(! Util::MapContains(template_map_, key));
        template_map_[key] = temp;

        // Create (empty) storage for the available instances.
        available_instances_[key] = AvailableList_();
    }

    /// Clears all templates that have been added. This should be called when
    /// reloading the scene before adding templates again.
    void ClearTemplates();

    /// Activates and returns an instance of the templated type of Feedback.
    /// Asserts if anything goes wrong.
    template <typename T> std::shared_ptr<T> Activate() {
        std::type_index key(typeid(T));
        ASSERT(Util::MapContains(template_map_, key));

        std::shared_ptr<T> instance;

        // Check for available storage.
        auto &avail = available_instances_[key];
        if (! avail.empty()) {
            instance = Util::CastToDerived<T>(avail.front());
            avail.pop_front();
        }
        else {
            // Nothing available. Create a new instance.
            instance = template_map_[key]->CloneTyped<T>(true);
        }
        ActivateInstance_(Util::CastToBase<Feedback>(instance));
        return instance;
    }

    /// Deactivates the given Feedback instance.
    template <typename T> void Deactivate(const std::shared_ptr<T> &instance) {
        DeactivateInstance_(Util::CastToBase<Feedback>(instance));

        // Add the instance to the available list.
        std::type_index key(typeid(T));
        ASSERT(Util::MapContains(available_instances_, key));
        available_instances_[key].push_front(instance);
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

    /// Activates an instance and adds it to the appropriate parent.
    void ActivateInstance_(const FeedbackPtr &instance);

    /// Deactivates an instance and removes it from the appropriate parent.
    void DeactivateInstance_(const FeedbackPtr &instance);
};

typedef std::shared_ptr<FeedbackManager> FeedbackManagerPtr;
