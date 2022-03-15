#pragma once

#include <unordered_map>
#include <vector>

#include "Enums/Action.h"
#include "Enums/ActionCategory.h"

/// The ActionMap class manages associations between the Action and
/// ActionCategory enums. It can return the ActionCategory that an Action
/// belongs to and also return all Action values that belong to an
/// ActionCategory.
class ActionMap {
  public:
    ActionMap();

    /// Returns the ActionCategory the given Action belongs to.
    ActionCategory GetCategoryForAction(Action action) const;

    /// Returns a list of Action values for the given ActionCategory.
    const std::vector<Action> & GetActionsInCategory(ActionCategory cat) const;

  private:
    /// Typedef for map from each ActionCategory to all Action values in it.
    typedef std::unordered_map<ActionCategory,
                               std::vector<Action>> CategoryMap_;

    /// Typedef for map from each Action to its ActionCategory.
    typedef std::unordered_map<Action, ActionCategory> ActionMap_;

    /// Maps each ActionCategory to all Action values in it.
    const CategoryMap_ category_map_;

    /// Maps each Action to its ActionCategory.
    const ActionMap_   action_map_;

    static CategoryMap_ BuildCategoryMap_();
    static ActionMap_   BuildActionMap_(const CategoryMap_ &category_map);
};
