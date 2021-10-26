#pragma once

#include <memory>
#include <vector>

#include "Math/Types.h"
#include "SG/Node.h"

namespace Parser { class Registry; }

/// A Board is a 2D rectangle that can be optionally moved and sized using
/// slider handles on the edges and corners.
class Board : public SG::Node {
  public:
    virtual void AddFields() override;

    /// Shows or hides slider handles used to move the Board. They are enabled
    /// by default.
    void EnableMove(bool enable);

    /// Shows or hides slider handles used to size the Board. They are enabled
    /// by default.
    void EnableSize(bool enable);

    /// Sets the size of the Board. The default size is 20x20.
    void SetSize(const Vector2f &size);

    /// Shows or hides the Board. This should be used instead of enabling or
    /// disabling traversal directly, as it sets up the Board first if
    /// necessary. Note that a Board is hidden by default.
    void Show(bool shown);

    /// Defines this to set up the canvas color.
    void PostSetUpIon() override;

  protected:
    Board();

  private:
    struct Parts_;
    std::unique_ptr<Parts_> parts_;

    Vector2f size_{ 20, 20 };
    bool is_move_enabled_   = true;
    bool is_size_enabled_ = true;

    /// Updates all of the parts of the Board for the first time or when
    /// anything changes. Finds them first if necessary.
    void UpdateParts_();

    /// Finds and stores all of the necessary parts.
    void FindParts_();

    void MoveActivated_(bool is_activation);
    void SizeActivated_(bool is_activation);
    void Move_();
    void Size_();

    friend class Parser::Registry;
};

typedef std::shared_ptr<Board> BoardPtr;
