#pragma once

#include "Math/Bevel.h"
#include "Panels/ToolPanel.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(BevelToolPanel);
DECL_SHARED_PTR(ProfilePane);
DECL_SHARED_PTR(SliderPane);

namespace Parser { class Registry; }

/// BevelToolPanel is a derived ToolPanel class that is used by the BevelTool
/// for interactive editing of one or more BeveledModel instances.
///
/// ReportChange() keys: "Profile", "Scale", "MaxAngle" (all drag-based, but
/// creation of a new profile point is immediate).
///
/// \ingroup Panels
class BevelToolPanel : public ToolPanel {
  public:
    /// Sets the initial Bevel to edit. This initializes the Panel for editing
    /// and sets up the sliders.
    void SetBevel(const Bevel &bevel);

    /// Returns the current bevel after possible editing.
    Bevel GetBevel() const;

    virtual bool CanGripHover() const { return true; }
    virtual ClickableWidgetPtr GetGripWidget(const Point2f &panel_point);

  protected:
    BevelToolPanel() {}

    virtual void InitInterface() override;

  private:
    bool was_bevel_set_ = false;
    bool is_dragging_   = false;

    // Parts.
    ProfilePanePtr profile_pane_;
    SliderPanePtr  scale_slider_;
    SliderPanePtr  angle_slider_;

    void Activate_(const Str &key, bool is_activation);
    void Change_(const Str &key);

    friend class Parser::Registry;
};
