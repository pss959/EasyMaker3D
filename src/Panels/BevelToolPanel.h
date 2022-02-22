#pragma once

#include <memory>

#include "Math/Bevel.h"
#include "Panes/ProfilePane.h"
#include "Panes/SliderPane.h"
#include "ToolPanel.h"

namespace Parser { class Registry; }

/// BevelToolPanel is a derived ToolPanel class thatis used by the BevelTool
/// for interactive editing of one or more BeveledModel instances.
///
/// ReportChange keys: "Profile", "Scale", "MaxAngle" (all drag-based, but
/// creation of a new profile point is immediate).
///
/// \ingroup Panels
class BevelToolPanel : public ToolPanel {
  public:
    /// Sets the initial Bevel to edit. This initializes the Panel for editing
    /// and sets up the sliders.
    void SetBevel(const Bevel &bevel);

    /// Returns the current bevel after possible editing.
    const Bevel & GetBevel() const { return bevel_; }

  protected:
    BevelToolPanel() {}

  private:
    Bevel bevel_;

    // Parts.
    // XXXX
    //ProfilePanePtr profile_pane_;
    //SliderPanePtr  scale_slider_;
    //SliderPanePtr  angle_slider_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<BevelToolPanel> BevelToolPanelPtr;
