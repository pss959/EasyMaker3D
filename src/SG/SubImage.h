#pragma once

#include "Math/Types.h"
#include "Memory.h"
#include "SG/Object.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(SubImage);

/// The SubImage class represents a rectangular part of an Image; it contains
/// texture coordinate scale and offset values used to refer to that part.
class SubImage : public Object {
  public:
    virtual bool IsNameRequired() const override { return true; }

    const Vector2f & GetTextureScale()  const { return texture_scale_; }
    const Vector2f & GetTextureOffset() const { return texture_offset_; }

  protected:
    SubImage() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Vector2f> texture_scale_{"texture_scale",   {1, 1}};
    Parser::TField<Vector2f> texture_offset_{"texture_offset", {0, 0}};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
