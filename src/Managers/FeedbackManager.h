#pragma once

#include <forward_list>
#include <memory>
#include <typeindex>
#include <unordered_map>

#include "Assert.h"
#include "Feedback/Feedback.h"
#include "SG/Typedefs.h"
#include "Util/General.h"

/// The FeedbackManager manages instances of derived Feedback classes so they
/// can be shared and reused by all Tools that need them.
///
/// \ingroup Managers
class FeedbackManager {
  public:
    /// The constructor is passed the SG::Node that is to be used as a parent
    /// for all active Feedback objects. It is assumed that this node defines
    /// the correct (stage) coordinate system.
    FeedbackManager(const SG::NodePtr &parent_node) :
        parent_node_(parent_node) {
        ASSERT(parent_node);
    }

    /// Stores a template for a specific type of Feedback. The template is used
    /// to create instances of that type.
    template <typename T> void AddTemplate(const std::shared_ptr<T> &temp) {
        std::type_index key(typeid(T));
        ASSERT(! Util::MapContains(template_map_, key));
        template_map_[key] = temp;

        // Create (empty) storage for the available instances.
        available_instances_[key] = AvailableList_();
    }

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
            instance =
                Util::CastToDerived<T>(template_map_[key]->CloneNode(true));
        }
        ASSERT(instance);
        instance->Activate();
        parent_node_->AddChild(instance);
        return instance;
    }

    /// Deactivates the given Feedback instance.
    template <typename T> void Deactivate(const std::shared_ptr<T> &instance) {
        ASSERT(instance);
        instance->Deactivate();
        parent_node_->RemoveChild(instance);

        // Add the instance to the available list.
        std::type_index key(typeid(T));
        ASSERT(Util::MapContains(available_instances_, key));
        available_instances_[key].push_front(instance);
    }

  private:
    /// Typedef for available instance storage. Use a forward_list to make
    /// insertion and removal easier - no need for random access.
    typedef std::forward_list<FeedbackPtr> AvailableList_;

    /// Node to add active Feedback instances to.
    SG::NodePtr parent_node_;

    /// This stores the template for each derived Feedback class, keyed by
    /// type_index.
    std::unordered_map<std::type_index, FeedbackPtr> template_map_;

    /// This stores the available instances for each type of Feedback, keyed by
    /// type_index.
    std::unordered_map<std::type_index, AvailableList_> available_instances_;
};

typedef std::shared_ptr<FeedbackManager> FeedbackManagerPtr;
