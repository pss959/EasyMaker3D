#pragma once

#include <string>

#include "Base/Memory.h"
#include "Math/Bevel.h"  // XXXX
#include "Panels/ToolPanel.h"

DECL_SHARED_PTR(TaperToolPanel);
DECL_SHARED_PTR(ProfilePane);
DECL_SHARED_PTR(SliderPane);

namespace Parser { class Registry; }

/// TaperToolPanel is a derived ToolPanel class that is used by the TaperTool
/// for interactive editing of one or more TaperedModel instances.
///
/// ReportChange keys: "Profile", "Scale", "MaxAngle" (all drag-based, but
/// creation of a new profile point is immediate).
///
/// \ingroup Panels
class TaperToolPanel : public ToolPanel {
  public:
    /// Sets the initial Taper to edit. This initializes the Panel for editing
    /// and sets up the sliders.
    void SetTaper(const Bevel &taper);

    /// Returns the current taper after possible editing.
    Bevel GetTaper() const;

    virtual bool CanGripHover() const { return true; }
    virtual ClickableWidgetPtr GetGripWidget(const Point2f &panel_point);

  protected:
    TaperToolPanel() {}

    virtual void CreationDone() override;

  private:
    bool was_taper_set_ = false;
    bool is_dragging_   = false;

    // Parts.
    ProfilePanePtr profile_pane_;
    SliderPanePtr  scale_slider_;
    SliderPanePtr  angle_slider_;

    void Activate_(const std::string &key, bool is_activation);
    void Change_(const std::string &key);

    friend class Parser::Registry;
};
