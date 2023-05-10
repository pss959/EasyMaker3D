#pragma once

#include <string>

#include "Base/Memory.h"
#include "Enums/Axis.h"
#include "Math/Taper.h"
#include "Panels/ToolPanel.h"

DECL_SHARED_PTR(TaperToolPanel);
DECL_SHARED_PTR(TaperProfilePane);
DECL_SHARED_PTR(SliderPane);

namespace Parser { class Registry; }

/// TaperToolPanel is a derived ToolPanel class that is used by the TaperTool
/// for interactive editing of one or more TaperedModel instances.
///
/// ReportChange keys: "Profile" (drag-based, but creation of a new profile
/// point is immediate) and "Axis" (immediate).
///
/// \ingroup Panels
class TaperToolPanel : public ToolPanel {
  public:
    /// Sets the initial Taper to edit.
    void SetTaper(const Taper &taper);

    /// Returns the current taper after possible editing.
    Taper GetTaper() const;

    virtual bool CanGripHover() const { return true; }
    virtual ClickableWidgetPtr GetGripWidget(const Point2f &panel_point);

  protected:
    TaperToolPanel() {}

    virtual void InitInterface() override;
    virtual void UpdateInterface() override;

  private:
    bool was_taper_set_ = false;
    bool is_dragging_   = false;

    TaperProfilePanePtr profile_pane_;

    /// Currently selected Axis.
    Axis axis_ = Axis::kY;

    void Activate_(const std::string &key, bool is_activation);
    void Change_(const std::string &key);

    // Converts an Axis from the application coordinate system (+Y up) to the
    // user/3D-printing coordinate system (+Z up).
    static Axis ToUserAxis(Axis axis);

    // Does the opposite of ToUserAxis().
    static Axis FromUserAxis(Axis axis);

    friend class Parser::Registry;
};
