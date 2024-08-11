//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Types.h"
#include "SG/Object.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(SubImage);

/// The SubImage class represents a rectangular part of an Image; it contains
/// texture coordinate scale and offset values used to refer to that part.
///
/// \ingroup SG
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
    Parser::TField<Vector2f> texture_scale_;
    Parser::TField<Vector2f> texture_offset_;
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
