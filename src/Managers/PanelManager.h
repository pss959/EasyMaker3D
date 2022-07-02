#pragma once

#include <unordered_map>

#include "Base/Memory.h"
#include "Panels/Panel.h"
#include "Util/Assert.h"
#include "Util/General.h"

DECL_SHARED_PTR(PanelManager);
namespace SG { class Scene; }

/// The PanelManager manages instances of all derived Panel classes. It
/// maintains a stack of Board instances; the top Board is used to display a
/// Panel.
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
    PanelPtr GetPanel(const std::string &name) const;

    /// Same as GetPanel(), but requires that the Panel is of the given derived
    /// type. Asserts if not found.
    template <typename T>
    std::shared_ptr<T> GetTypedPanel(const std::string &name) const {
        auto panel = Util::CastToDerived<T>(GetPanel(name));
        ASSERT(panel);
        return panel;
    }

  private:
    typedef std::unordered_map<std::string, PanelPtr> PanelMap_;

    /// Maps panel name to panel instance.
    PanelMap_ panel_map_;
};
