#pragma once

#include <memory>
#include <stack>
#include <unordered_map>

#include "Items/Board.h"
#include "Panels/Panel.h"
#include "SG/Typedefs.h"

/// The PanelManager manages instances of all derived Panel classes.
///
/// \ingroup Managers
class PanelManager {
  public:
    /// Sets the Board to use to display and interact with panels.
    void SetBoard(const BoardPtr &board) { board_ = board; }

    /// Finds all necessary panel instances in the given Scene and sets them
    /// up with the given Panel::Context.
    void FindPanels(const SG::Scene &scene, const Panel::ContextPtr &context);

    /// Opens the named panel by attaching it to the Board and displaying the
    /// Board. Asserts if the name is not known.
    void OpenPanel(const std::string &panel_name);

  private:
    typedef std::unordered_map<std::string, PanelPtr> PanelMap_;

    /// Maps panel name to panel instance.
    PanelMap_ panel_map_;

    /// Board used to display and interact with panels.
    BoardPtr  board_;

    /// This saves the stack of Panels open for the Board. It is used to
    /// restore Panels when requested.
    std::stack<PanelPtr> open_panels_;

    /// Shows the given panel.
    void ShowPanel_(const PanelPtr &panel);

    /// This is invoked when a Panel is closed by user interaction.
    void PanelClosed_(Panel::CloseReason reason, const std::string &result);

};

typedef std::shared_ptr<PanelManager> PanelManagerPtr;
