//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <concepts>
#include <unordered_map>

#include "Panels/Panel.h"
#include "Util/Assert.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(PanelManager);
namespace SG { class Scene; }

/// The PanelManager manages instances of all derived Panel classes, allowing
/// any instance to be accessed by name. It is used by the BoardManager.
///
/// \ingroup Managers
class PanelManager {
  public:
    /// Clears all scene-related state in the manager.
    void Reset();

    /// Finds all necessary panel instances in the given Scene and sets them
    /// up with the given Panel::Context.
    void FindAllPanels(const SG::Scene &scene,
                       const Panel::ContextPtr &context);

    /// Returns the named Panel. Asserts if the name is not known.
    PanelPtr GetPanel(const Str &name) const;

    /// Same as GetPanel(), but requires that the Panel is of the given derived
    /// type. Asserts if not found.
    template <typename T>
    std::shared_ptr<T> GetTypedPanel(const Str &name) const {
        static_assert(std::derived_from<T, Panel> == true);
        auto panel = std::dynamic_pointer_cast<T>(GetPanel(name));
        ASSERTM(panel, "No typed panel named " + name);
        return panel;
    }

  private:
    using PanelMap_ = std::unordered_map<Str, PanelPtr>;

    /// Maps panel name to panel instance.
    PanelMap_ panel_map_;
};
