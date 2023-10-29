#pragma once

#include <string>

#include "Math/Profile.h"
#include "Math/Types.h"
#include "Panels/ToolPanel.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(CheckboxPane);
DECL_SHARED_PTR(ExtrudedToolPanel);
DECL_SHARED_PTR(ProfilePane);
DECL_SHARED_PTR(SliderPane);
DECL_SHARED_PTR(TextInputPane);

namespace Parser { class Registry; }

/// ExtrudedToolPanel is a derived ToolPanel class that is used by the
/// ExtrudedTool for interactive editing of one or more ExtrudedModel
/// instances.
///
/// ReportChange() keys: "Profile" (immediate for click to create a new point
/// or to set to a regular polygon, drag-based otherwise).
///
/// \ingroup Panels
class ExtrudedToolPanel : public ToolPanel {
  public:
    /// Sets the initial Profile to edit.
    void SetProfile(const Profile &profile);

    /// Returns the current Profile after possible editing.
    const Profile & GetProfile() const;

    /// Sets the precision to use for snapping when enabled by the checkbox.
    void SetPrecision(const Vector2f &xy_precision) {
        precision_ = xy_precision;
        UpdatePrecision_();
    }

    virtual bool CanGripHover() const { return true; }
    virtual ClickableWidgetPtr GetGripWidget(const Point2f &panel_point);

protected:
    ExtrudedToolPanel() {}

    virtual void InitInterface() override;

  private:
    Vector2f precision_{0, 0};
    bool     is_dragging_ = false;

    // Parts.
    CheckboxPanePtr  snap_checkbox_;
    ProfilePanePtr   profile_pane_;
    TextInputPanePtr sides_text_;
    SliderPanePtr    sides_slider_;

    void UpdatePrecision_();
    bool ValidateSidesText_(const Str &text);
    void UpdateSidesFromSlider_(size_t sides);
    void SetToPolygon_();
    void Activate_(const Str &key, bool is_activation);
    void Change_(const Str &key);

    friend class Parser::Registry;
};
