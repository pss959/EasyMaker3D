#pragma once

#include "Agents/BoardAgent.h"
#include "Base/Memory.h"
#include "Items/Grippable.h"
#include "Items/Touchable.h"
#include "Math/Types.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(Board);
DECL_SHARED_PTR(Panel);

/// A Board is a 2D rectangle that can be optionally moved and sized using
/// slider handles on the edges and corners. It is derived from Grippable
/// because it allows grip interaction with the slider handles. It is also
/// derived from Touchable because it supports touch interaction on its
/// interactive Panes.
///
/// Each Board maintains a stack of Panel instances that are active within the
/// Board.
///
/// \ingroup Panels
class Board : public Grippable, public Touchable {
  public:
    /// Defines the visibility behavior of a Board.
    enum class Behavior {
        /// The Board is permanently visible; it has no effect on other Boards
        /// and no other Boards affect it.
        kPermanent,
        /// The Board becomes visible when shown and hides all other Boards
        /// that are not Behavior::kPermanent. (Default)
        kReplaces,
        /// The Board becomes visible when shown, but leaves all all other
        /// Boards in their current state.
        kAugments,
    };

    /// Returns the Behavior for the Board.
    Behavior GetBehavior() const { return behavior_; }

    /// Returns a flag indicating whether the Board is floating, meaning that
    /// it should be positioned by the Application to be visible. The default
    /// is false.
    bool IsFloating() const { return is_floating_; }

    /// Returns an offset vector that is applied to the position chosen for the
    /// Board when set up for touch interaction.
    const Vector3f & GetTouchOffset() const { return touch_offset_; }

    /// \name Panel Management
    ///@{

    /// Sets a Panel to display in the board. If the ResultFunc is not null, it
    /// is invoked when the Panel is closed. This asserts if the Board already
    /// has a Panel set.
    void SetPanel(const PanelPtr &panel,
                  const BoardAgent::ResultFunc &result_func = nullptr);

    /// Pushes a Panel to display in the board. The ResultFunc is invoked when
    /// the Panel is closed (popped). This asserts if the Board is not already
    /// showing a Panel. This asserts if the behavior is Behavior::kPermanent.
    void PushPanel(const PanelPtr &panel,
                   const BoardAgent::ResultFunc &result_func);

    /// Pops the current Panel, passing the given string to the result function
    /// (if any). Returns true if there are more Panels left on the
    /// stack. Asserts if there is no current Panel.
    bool PopPanel(const Str &result);

    /// Returns the Panel currently displayed in the board or null if there is
    /// none.
    PanelPtr GetCurrentPanel() const;

    /// Sets a scale factor to use for the Panel instead of the default value
    /// of TK::kPanelToWorld. This allows tweaking the content scaling to
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
    virtual void UpdateForRenderPass(const Str &pass_name) override;

    /// Sets up the Board for use with controller touches in VR. The position
    /// of the camera is supplied so the Board can scale and position itself
    /// relative to it. Note that the offset returned by GetOffset() is applied
    /// after positioning. If the origin is passed in, this disables touch
    /// processing.
    void SetUpForTouch(const Point3f &cam_pos);

    /// Sets the Board to orient for the given view direction vector. This has
    /// no effect if the Board is set up for touch.
    void SetOrientation(const Vector3f &view_dir);

    /// Sets the Board's position. This has no effect if the Board is set up
    /// for touch.
    void SetPosition(const Point3f &pos);

    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual void SetPath(const SG::NodePath &path) override;
    virtual const SG::Node * GetGrippableNode() const override;
    virtual void UpdateGripInfo(GripInfo &info) override;
    virtual void ActivateGrip(Hand hand, bool is_active) override;

    // ------------------------------------------------------------------------
    // Touchable interface.
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
    Parser::EnumField<Behavior> behavior_;
    Parser::TField<bool>        is_floating_;
    Parser::TField<Vector3f>    touch_offset_;
    ///@}

    friend class Parser::Registry;
    friend class BoardAgent;

    /// Hide this so SetOrientation() must be used.
    using Grippable::SetRotation;
    /// Hide this so SetPosition() must be used.
    using Grippable::SetTranslation;
};
