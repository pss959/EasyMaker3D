#pragma once

#include <memory>

#include "Panes/Pane.h"

namespace Parser { class Registry; }

/// SolidPane is a derived Pane that adds a solid rectangle with an optional
/// border to give the Pane a physical presence.
class SolidPane : public Pane {
  public:
    virtual void AddFields() override;

    virtual void PreSetUpIon() override;
    virtual void PostSetUpIon() override;

  protected:
    SolidPane() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Color> color_{"color"};
    Parser::TField<Color> border_color_{"border_color"};
    Parser::TField<float> border_width_{"border_width"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<SolidPane> SolidPanePtr;
