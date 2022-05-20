#pragma once

#include <string>

#include "Base/Memory.h"
#include "Math/Profile.h"
#include "Math/Types.h"
#include "ToolPanel.h"

DECL_SHARED_PTR(ProfilePane);
DECL_SHARED_PTR(SliderPane);
DECL_SHARED_PTR(RevSurfToolPanel);

namespace Parser { class Registry; }

/// RevSurfToolPanel is a derived ToolPanel class that is used by the
/// RevSurfTool for interactive editing of one or more RevSurfModel instances.
///
/// ReportChange keys: "Profile", "SweepAngle" (both drag-based, but creation
/// of a new profile point is immediate).
///
/// \ingroup Panels
class RevSurfToolPanel : public ToolPanel {
  public:
    /// Sets the initial Profile to edit.
    void SetProfile(const Profile &profile);

    /// Returns the current Profile after possible editing.
    const Profile & GetProfile() const;

    /// Sets the initial sweep angle.
    void SetSweepAngle(const Anglef &sweep_angle);

    /// Returns the sweep angle after possible editing.
    Anglef GetSweepAngle() const;

  protected:
    RevSurfToolPanel() {}

    virtual void CreationDone() override;

  private:
    bool is_dragging_ = false;

    // Parts.
    ProfilePanePtr profile_pane_;
    SliderPanePtr  sweep_angle_slider_;

    void Activate_(const std::string &key, bool is_activation);
    void Change_(const std::string &key);

    friend class Parser::Registry;
};
