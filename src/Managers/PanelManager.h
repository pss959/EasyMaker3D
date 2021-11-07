#pragma once

#include <memory>
#include <unordered_map>

#include "Items/Board.h"
#include "Panels/SessionPanel.h"
#include "SG/Typedefs.h"

/// The PanelManager manages instances of all derived Panel classes.
///
/// \ingroup Managers
class PanelManager {
  public:
    /// Sets the Board to use to display and interact with panels.
    void SetBoard(const BoardPtr &board) { board_ = board; }

    /// Finds all necessary panel instances in the given Scene.
    void FindPanels(const SG::Scene &scene);

    /// Activates the named panel. Asserts if it does not exist.
    void Activate(const std::string &panel_name);

  private:
    typedef std::unordered_map<std::string, PanelPtr> PanelMap_;

    /// Maps panel name to panel instance.
    PanelMap_ panel_map_;

    /// Board used to display and interact with panels.
    BoardPtr        board_;
};

typedef std::shared_ptr<PanelManager> PanelManagerPtr;
