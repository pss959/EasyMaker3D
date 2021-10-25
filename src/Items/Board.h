#pragma once

#include <memory>
#include <vector>

#include "Math/Types.h"
#include "SG/Node.h"

namespace Parser { class Registry; }

/// A Board is a 2D rectangle that can be optionally moved and resized using
/// slider handles on the edges and corners.
class Board : public SG::Node {
  public:
    virtual void AddFields() override;

    /// Enables or disables handles used to move the Board. They are enabled by
    /// default.
    void EnableMoveHandles(bool enable);

    /// Enables or disables handles used to resize the Board. They are enabled
    /// by default.
    void EnableResizeHandles(bool enable);

    /// Sets the size of the Board. The default size is 20x20.
    void SetSize(const Vector2f &size);

    /// Shows or hides the Board. This should be used instead of enabling or
    /// disabling traversal directly, as it sets up the Board first if
    /// necessary. Note that a Board is hidden by default.
    void Show(bool shown);

    void PostSetUpIon() override;

  protected:
    Board();

  private:
    /// Enum representing a side of the board.
    enum class Side_ { kLeft, kRight, kBottom, kTop };

    /// Enum representing a corner of the board.
    enum class Corner_ { kBottomLeft, kBottomRight, kTopLeft, kTopRight };

    struct Parts_;
    std::unique_ptr<Parts_> parts_;

    Vector2f size_{ 20, 20 };
    bool are_move_handles_enabled_   = true;
    bool are_resize_handles_enabled_ = true;

    /// Updates all of the parts of the Board for the first time or when
    /// anything changes. Finds them first if necessary.
    void UpdateParts_();

    /// Finds and stores all of the necessary parts.
    void FindParts_();

    void MoveSliderActivated_(Side_ side, bool is_activation);

    friend class Parser::Registry;
};

typedef std::shared_ptr<Board> BoardPtr;
