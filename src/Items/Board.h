#pragma once

#include <memory>
#include <vector>

#include "Items/Grippable.h"
#include "Math/Types.h"
#include "Panels/Panel.h"

namespace Parser { class Registry; }

/// A Board is a 2D rectangle that can be optionally moved and sized using
/// slider handles on the edges and corners. It is derived from Grippable
/// because it allows grip interaction with the slider handles.
class Board : public Grippable {
  public:
    /// Shows or hides slider handles used to move the Board. They are enabled
    /// by default.
    void EnableMove(bool enable);

    /// Shows or hides slider handles used to size the Board. They are enabled
    /// by default.
    void EnableSize(bool enable);

    /// Sets the size of the Board, which is (0,0) until a Panel is set and
    /// reports its size.
    void SetSize(const Vector2f &size);

    /// Returns the current size of the Board.
    const Vector2f & GetSize() const;

    /// Sets the Panel to display in the board.
    void SetPanel(const PanelPtr &panel);

    /// Returns the Panel displayed in the board.
    const PanelPtr & GetPanel() const;

    /// Shows or hides the Board. This should be used instead of enabling or
    /// disabling traversal directly, as it sets up the Board first if
    /// necessary. Note that a Board is hidden by default.
    void Show(bool shown);

    /// Returns true if the Board is currently shown.
    bool IsShown() const { return IsEnabled(Flag::kTraversal); }

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

  private:
    class Impl_;   // This does most of the work.
    std::unique_ptr<Impl_> impl_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<Board> BoardPtr;
