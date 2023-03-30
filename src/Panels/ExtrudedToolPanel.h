#pragma once

#include <string>

#include "Base/Memory.h"
#include "Math/Profile.h"
#include "Math/Types.h"
#include "Panels/ToolPanel.h"

DECL_SHARED_PTR(CheckboxPane);
DECL_SHARED_PTR(ProfilePane);
DECL_SHARED_PTR(ExtrudedToolPanel);

namespace Parser { class Registry; }

/// ExtrudedToolPanel is a derived ToolPanel class that is used by the
/// ExtrudedTool for interactive editing of one or more ExtrudedModel
/// instances.
///
/// ReportChange keys: "Profile", "XXXX". (Profile is drag-based, but creation
/// of a new profile point is immediate; XXXX is immediate).
///
/// XXXX Add button to generate regular polygon.
/// XXXX Add slider to rotate profile ? Or just rely on RotateTool?
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

    virtual void CreationDone() override;

  private:
    Vector2f precision_{0, 0};
    bool     is_dragging_ = false;

    // Parts.
    CheckboxPanePtr snap_checkbox_;
    ProfilePanePtr  profile_pane_;

    void UpdatePrecision_();
    void Activate_(const std::string &key, bool is_activation);
    void Change_(const std::string &key);

    friend class Parser::Registry;
};
