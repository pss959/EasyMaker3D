#pragma once

#include <memory>
#include <stack>
#include <unordered_map>

#include "Items/Board.h"
#include "Panels/Panel.h"
#include "Panels/PanelHelper.h"
#include "SG/Typedefs.h"

/// The PanelManager manages instances of all derived Panel classes.
///
/// \ingroup Managers
class PanelManager : public PanelHelper {
  public:
    /// Clears all scene-related state in the manager.
    void Reset();

    /// Sets the Board to use to display and interact with panels.
    void SetBoard(const BoardPtr &board) { board_ = board; }

    /// Finds all necessary panel instances in the given Scene and sets them
    /// up with the given Panel::Context.
    void FindPanels(const SG::Scene &scene, const Panel::ContextPtr &context);

    /// Opens the named panel by attaching it to the Board and displaying the
    /// Board. Asserts if the name is not known. Returns the opened Panel.
    PanelPtr OpenPanel(const std::string &panel_name);

    // ------------------------------------------------------------------------
    // PanelHelper interface.
    // ------------------------------------------------------------------------
    virtual void Close(const std::string &result) override;
    virtual void Replace(const std::string &panel_name,
                         const InitFunc &init_func,
                         const ResultFunc &result_func) override;

  private:
    /// Information about a Panel that has been replaced.
    struct PanelInfo_ {
        PanelPtr   panel;
        ResultFunc result_func;
    };

    typedef std::unordered_map<std::string, PanelPtr> PanelMap_;

    /// Maps panel name to panel instance.
    PanelMap_ panel_map_;

    /// Board used to display and interact with panels.
    BoardPtr  board_;

    /// Stack of Panels that have been replaced, allowing them to be restored
    /// when the replacement Panel is closed.
    std::stack<PanelInfo_> panel_stack_;

    /// Shows the given panel.
    void ShowPanel_(const PanelPtr &panel);

    /// Actually opens the named panel. If old_panel is not null, it is given a
    /// chance to initialize the new panel that replaces it. This is used
    /// primarily to set up a FilePanel for a specific target.
    void OpenPanel_(const std::string &panel_name, const PanelPtr &old_panel);
};

typedef std::shared_ptr<PanelManager> PanelManagerPtr;
