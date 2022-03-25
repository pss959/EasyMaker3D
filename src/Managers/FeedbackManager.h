#pragma once

#include "Feedback/Feedback.h"
#include "Managers/InstanceManager.h"
#include "Math/Types.h"
#include "Memory.h"
#include "SG/NodePath.h"
#include "Util/Assert.h"
#include "Util/General.h"

DECL_SHARED_PTR(FeedbackManager);
namespace SG { DECL_SHARED_PTR(Node); }

/// The FeedbackManager manages instances of derived Feedback classes so they
/// can be shared and reused by all Tools that need them.
///
/// \ingroup Managers
class FeedbackManager : public InstanceManager {
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

    /// Sets the path to the Stage for coordinate conversions.
    void SetPathToStage(const SG::NodePath &path) { path_to_stage_ = path; }

    /// Activates and returns an instance of the templated type of Feedback.
    /// Asserts if anything goes wrong.
    template <typename T> std::shared_ptr<T> Activate() {
        std::shared_ptr<T> instance = Acquire<T>();
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
        DeactivateInstance_(Util::CastToBase<Feedback>(instance));
        Release<T>(instance);
    }

    /// Same as Deactivate(), but looks up the Feedback instance by key.
    /// Asserts if it is not found.
    template <typename T> void DeactivateWithKey(const std::string &key) {
        auto instance = FindActiveInstance_(key);
        RemoveActiveInstance_(key, instance);
        ASSERT(Util::CastToDerived<T>(instance));
        Deactivate(Util::CastToDerived<T>(instance));
    }

  private:
    /// Parent Node for active Feedback instances that operate in world
    /// coordinates.
    SG::NodePtr world_parent_node_;

    /// Parent Node for active Feedback instances that operate in stage
    /// coordinates.
    SG::NodePtr stage_parent_node_;

    /// Function to invoke to get the scene bounds.
    SceneBoundsFunc scene_bounds_func_;

    /// Path to stage, used to orient text feedback.
    SG::NodePath path_to_stage_;

    /// This stores the active instances for all types of Feedback, keyed by
    /// user-defined string.
    std::unordered_map<std::string, FeedbackPtr> active_instances_;

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
