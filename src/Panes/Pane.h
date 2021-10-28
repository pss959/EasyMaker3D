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

    /// All derived classes must define this to set the size of the Pane,
    /// updating all subpanes as necessary.
    virtual void SetSize(const Vector2f &size) = 0;

    /// Returns the current size of the Pane.
    const Vector2f & GetSize() const { return size_; }

    /// Returns the minimum size of the Pane.
    const Vector2f & GetMinSize() const { return min_size_; }

    /// Returns true if the width of this Pane should respond to size changes.
    bool IsWidthResizable() const { return resize_width_; }

    /// Returns true if the height of this Pane should respond to size changes.
    bool IsHeightResizable() const { return resize_height_; }

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

    Vector2f size_{0, 0};
};

typedef std::shared_ptr<Pane> PanePtr;
