#pragma once

#include "Math/Types.h"
#include "Memory.h"
#include "SG/Node.h"

DECL_SHARED_PTR(PaneBackground);

namespace Parser { class Registry; }

/// PaneBackground is used to add a colored background to any Pane.
///
/// \ingroup Items
class PaneBackground : public SG::Node {
  public:
    virtual void PostSetUpIon() override;

  protected:
    PaneBackground();

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Color> color_{"color"};
    ///@}

    friend class Parser::Registry;
};
