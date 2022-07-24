#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "SG/Node.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(PaneBorder);

/// PaneBorder is used to add a colored border to any Pane.
///
/// \ingroup Items
class PaneBorder : public SG::Node {
  public:
    virtual void PostSetUpIon() override;

  protected:
    PaneBorder();

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Color> color_;
    Parser::TField<float> width_;
    ///@}

    friend class Parser::Registry;
};
