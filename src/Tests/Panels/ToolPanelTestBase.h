#pragma once

#include "Tests/Panels/PanelTestBase.h"

class ToolPanel;

/// Derived PanelTestBase class that is helpful for testing Tool-based Panels
/// (derived from ToolPanel).
///
/// \ingroup Tests
class ToolPanelTestBase : public PanelTestBase {
  protected:
    /// This struct stores information about the last change reported by the
    /// Panel via the GetInteraction() Notifier.
    struct ChangeInfo {
        size_t count = 0;  ///< Number of changes reported.
        Str    name;       ///< Name of the last change reported.
        Str    type;       ///< Type (ToolPanel::InteractionType as string).
    };

    explicit ToolPanelTestBase(bool need_text = false) :
        PanelTestBase(need_text) {}

    /// Adds an Observer to the ToolPanel's GetInteraction() Notifier that
    /// saves information about the last change.
    void ObserveChanges(ToolPanel &panel);

    /// Returns a ChangeInfo struct containing info about the most recent
    /// change reported by the Panel.
    const ChangeInfo & GetChangeInfo() const { return last_change_info_; }

  private:
    ChangeInfo last_change_info_;
};
