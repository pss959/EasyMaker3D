#pragma once

#pragma once

#include <memory>

#include "SG/Node.h"

/// Pane is an abstract base class for a rectangular 2D element that lives
/// inside a Panel. The Pane class manages automatic sizing and placement.
class Pane : public SG::Node {
  public:
    virtual void AddFields() override;

  protected:
    Pane() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector2f> min_size_{"min_size"};
    Parser::TField<bool>     resize_width_{"resize_width"};
    Parser::TField<bool>     resize_height_{"resize_height"};
    Parser::TField<Color>    color_{"color"};
    Parser::TField<Color>    border_color_{"border_color"};
    Parser::TField<float>    border_width_{"border_width"};
    ///@}
};

typedef std::shared_ptr<Pane> PanePtr;
