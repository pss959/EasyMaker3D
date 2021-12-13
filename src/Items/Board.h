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
    const Vector2f & GetSize() const { return size_; }

    /// Returns the current size of the Board.
    void SetSize(const Vector2f &size);

    /// Sets the Panel to display in the board.
    void SetPanel(const PanelPtr &panel);

    /// Returns the Panel displayed in the board.
    const PanelPtr & GetPanel() const { return panel_; }

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
    virtual bool IsGrippableEnabled() const override {
        return IsShown() && (is_move_enabled_ || is_size_enabled_);
    }
    virtual void UpdateGripInfo(GripInfo &info) override;

  protected:
    Board();

  private:
    struct Parts_;
    std::unique_ptr<Parts_> parts_;

    PanelPtr panel_;
    Vector2f size_{0, 0};
    bool is_move_enabled_   = true;
    bool is_size_enabled_ = true;

    Vector3f start_pos_;
    Vector3f start_scale_;

    bool may_need_resize_ = true;

    /// This saves the part that was last hovered in UpdateGripInfo() so that
    /// the Size_() function knows what to do.
    SG::NodePtr grip_hovered_part_;

    /// Set to true if grip_hovered_part_ is in the size slider as opposed to
    /// the move slider.
    bool is_size_hovered_ = false;

    /// This saves the part that is currently being dragged so that it is the
    /// only one that is hovered.
    SG::NodePtr grip_dragged_part_;

    /// Finds and stores all of the necessary parts.
    void FindParts_();

    /// Updates all of the parts of the Board for the first time or when
    /// anything changes. Finds them first if necessary.
    void UpdateParts_();

    void UpdateHandlePositions_();
    void MoveActivated_(bool is_activation);
    void SizeActivated_(bool is_activation);
    void Move_();
    void Size_();

    void UpdateSize_(const Vector2f &new_size, bool update_parts);
    void ScaleCanvasAndFrame_();

    SG::NodePtr GetBestGripHoverPart_(const Event &event, bool &is_size);

    friend class Parser::Registry;
};

typedef std::shared_ptr<Board> BoardPtr;
