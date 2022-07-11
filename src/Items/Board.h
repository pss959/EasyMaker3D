#pragma once

#include "Base/Memory.h"
#include "Items/Grippable.h"
#include "Math/Types.h"
#include "Panels/PanelHelper.h"
#include "Widgets/ITouchable.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(Board);
DECL_SHARED_PTR(Panel);

/// A Board is a 2D rectangle that can be optionally moved and sized using
/// slider handles on the edges and corners. It is derived from Grippable
/// because it allows grip interaction with the slider handles. It is also
/// derived from ITouchable because it supports touch interaction on its
/// interactive Panes.
///
/// Each Board maintains a stack of Panel instances that are active within the
/// Board.
///
/// \ingroup Items
class Board : public Grippable, public ITouchable {
  public:
    /// Defines the visibility behavior of a Board.
    enum class Behavior {
        /// The Board is permanently visible; it has no effect on other Boards
        /// and no other Boards affect it.
        kPermanent,
        /// The Board becomes visible when shown and hides all other Boards
        /// that are not Behavior::kPermanent.
        kReplaces,
        /// The Board becomes visible when shown, but leaves all all other
        /// Boards in their current state. (Default)
        kAugments,
    };

    /// Returns the Behavior for the Board.
    Behavior GetBehavior() const { return behavior_; }

    /// Returns a flag indicating whether the Board is floating, meaning that
    /// it should be positioned by the Application to be visible.
    bool IsFloating() const { return is_floating_; }

    /// \name Panel Management
    ///@{

    /// Sets a Panel to display in the board. This asserts if the Board already
    /// has a Panel set.
    void SetPanel(const PanelPtr &panel);

    /// Pushes a Panel to display in the board. The ResultFunc is invoked when
    /// the Panel is closed (popped). This asserts if the Board is not already
    /// showing a Panel.
    void PushPanel(const PanelPtr &panel,
                   const PanelHelper::ResultFunc &result_func);

    /// Pops the current Panel, passing the given string to the result function
    /// (if any). Asserts if there is no current Panel.
    void PopPanel(const std::string &result);

    /// Returns the Panel currently displayed in the board or null if there is
    /// none.
    PanelPtr GetCurrentPanel() const;

    /// Sets a scale factor to use for the Panel instead of the default value
    /// of Defaults::kPanelToWorld. This allows tweaking the content scaling to
    /// account for Board distance.
    void SetPanelScale(float scale);

    ///@}

    /// Shows or hides the Board. This should be used instead of enabling or
    /// disabling traversal directly, as it sets up the Board first if
    /// necessary. Note that a Board is hidden by default.
    void Show(bool shown);

    /// Returns true if the Board is currently shown.
    bool IsShown() const { return IsEnabled(); }

    /// Defines this to set up the canvas color.
    void PostSetUpIon() override;

    /// Defines this to resize if necessary.
    virtual void UpdateForRenderPass(const std::string &pass_name) override;

    /// Sets the VR camera position. This tells the Board to scale and
    /// position itself relative to this position to enable touch interaction.
    void SetVRCameraPosition(const Point3f &cam_pos);

    /// Sets an additional Z offset to position to relative to the VR camera.
    /// It is 0 by default.
    void SetVRCameraZOffset(float offset);

    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual const SG::Node * GetGrippableNode() const override;
    virtual void UpdateGripInfo(GripInfo &info) override;
    virtual void ActivateGrip(Hand hand, bool is_active) override;

    // ------------------------------------------------------------------------
    // ITouchable interface.
    // ------------------------------------------------------------------------
    virtual WidgetPtr GetTouchedWidget(const Point3f &touch_pos,
                                       float radius) const override;

  protected:
    Board();

    virtual void AddFields() override;
    virtual void CreationDone() override;

    /// Redefines this to update the Panel size first if necessary.
    virtual Bounds UpdateBounds() const override;

  private:
    class Impl_;   // This does most of the work.
    std::unique_ptr<Impl_> impl_;

    /// \name Parsed Fields
    ///@{
    Parser::EnumField<Behavior> behavior_{"behavior", Behavior::kReplaces};
    Parser::TField<bool>        is_floating_{"is_floating", false};
    ///@}

    friend class Parser::Registry;
    friend class PanelHelper;
};
