#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "SG/Object.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(NamedColor);

/// A NamedColor object associates a Color with a name (the name of the
/// instance). This is used to maintain a map of names to Colors for a Scene.
///
/// \ingroup SG
class NamedColor : public Object {
  public:
    virtual bool IsNameRequired() const override { return true; }

    /// Returns the Color.
    const Color & GetColor() const { return color_; }

  protected:
    NamedColor() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Color> color_{"color"};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
