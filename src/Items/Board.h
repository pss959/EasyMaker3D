#pragma once

#include <vector>

#include "Base/Memory.h"
#include "Items/Grippable.h"
#include "Math/Types.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(Board);
DECL_SHARED_PTR(Panel);

/// A Board is a 2D rectangle that can be optionally moved and sized using
/// slider handles on the edges and corners. It is derived from Grippable
/// because it allows grip interaction with the slider handles.
///
/// \ingroup Items
class Board : public Grippable {
  public:
    /// Sets the Panel to display in the board.
    void SetPanel(const PanelPtr &panel);

    /// Sets a scale factor to use for the Panel instead of the default value
    /// of Defaults::kPanelToWorld. This allows tweaking the content scaling to
    /// account for Board distance.
    void SetPanelScale(float scale);

    /// Returns the Panel displayed in the board.
    const PanelPtr & GetPanel() const;

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

    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual bool IsGrippableEnabled() const override;
    virtual void UpdateGripInfo(GripInfo &info) override;
    virtual void ActivateGrip(Hand hand, bool is_active) override;

  protected:
    Board();

    /// Redefines this to update the Panel size first if necessary.
    virtual Bounds UpdateBounds() const override;

  private:
    class Impl_;   // This does most of the work.
    std::unique_ptr<Impl_> impl_;

    friend class Parser::Registry;
};
