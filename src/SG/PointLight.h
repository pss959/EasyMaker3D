//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Color.h"
#include "Math/Types.h"
#include "SG/Object.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(PointLight);

/// A PointLight object represents a point light source. It has no Ion
/// equivalent.
///
/// \ingroup SG
class PointLight : public Object {
  public:
    const Point3f  & GetPosition()  const { return position_;     }
    const Color    & GetColor()     const { return color_;        }
    bool             CastsShadows() const { return cast_shadows_; }

  protected:
    PointLight() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Point3f>  position_;
    Parser::TField<Color>    color_;
    Parser::TField<bool>     cast_shadows_;
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
