#pragma once

#include <memory>

#include "Math/Types.h"
#include "SG/Node.h"

namespace Parser { class Registry; }

/// PaneBackground is used to add a colored background to any Pane.
class PaneBackground : public SG::Node {
  public:
    virtual void AddFields() override;

    virtual void PostSetUpIon() override;

  protected:
    PaneBackground();

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Color> color_{"color"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<PaneBackground> PaneBackgroundPtr;
