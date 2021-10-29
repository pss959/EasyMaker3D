#pragma once

#pragma once

#include <memory>

#include "Math/Types.h"
#include "SG/Node.h"

/// Pane is an abstract base class for a rectangular 2D element that lives
/// inside a Panel. The Pane class manages automatic sizing and placement.
class Pane : public SG::Node {
  public:
    virtual void AddFields() override;

    /// Sets the size of the pane. Derived classes may add other behavior.
    virtual void SetSize(const Vector2f &size);

    /// Returns the current size of the Pane.
    const Vector2f & GetSize() const { return size_; }

    /// Returns the minimum size of the Pane (in stage coordinate units).
    const Vector2f & GetMinSize() const { return min_size_; }

    /// Returns true if the width of this Pane should respond to size changes.
    bool IsWidthResizable() const { return resize_width_; }

    /// Returns true if the height of this Pane should respond to size changes.
    bool IsHeightResizable() const { return resize_height_; }

    /// Sets the Pane's rectangle within its parent in the range [0,1] in both
    /// dimensions. This also sets the scale and translation in the Pane so
    /// that it has the correct size and position relative to the parent.
    void SetRectInParent(const Range2f &rect);

    /// Returns the Pane's rectangle within its parent.
    const Range2f & GetRectInParent() const { return rect_in_parent_; }

  protected:
    Pane() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector2f> min_size_{"min_size", {1, 1}};
    Parser::TField<bool>     resize_width_{"resize_width", true};
    Parser::TField<bool>     resize_height_{"resize_height", true};
    Parser::TField<Color>    color_{"color"};
    Parser::TField<Color>    border_color_{"border_color"};
    Parser::TField<float>    border_width_{"border_width"};
    ///@}

    /// Size of this pane in world coordinates.
    Vector2f size_{0, 0};

    /// Relative size and position of this pane within its parent in the range
    /// [0,1] in both dimensions.
    Range2f  rect_in_parent_{{0,0},{1,1}};
};

typedef std::shared_ptr<Pane> PanePtr;
