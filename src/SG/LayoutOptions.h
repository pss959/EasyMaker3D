//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <ion/text/layout.h>

#include "Math/Types.h"
#include "SG/Object.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(LayoutOptions);

/// A LayoutOptions object wraps an Ion LayoutOptions struct. Note that the
/// target_size and target_point values are not settable, since the TextNode
/// expects them to have default settings.
///
/// \ingroup SG
class LayoutOptions : public Object {
  public:
    using HAlignment = ion::text::HorizontalAlignment;
    using VAlignment = ion::text::VerticalAlignment;

    HAlignment       GetHAlignment()    const { return halignment_;    }
    VAlignment       GetVAlignment()    const { return valignment_;    }
    float            GetLineSpacing()   const { return line_spacing_;  }
    float            GetGlyphSpacing()  const { return glyph_spacing_; }
    bool IsUsingMetricsBasedAlignment() const { return use_metrics_;   }

    void SetHAlignment(HAlignment alignment);
    void SetVAlignment(VAlignment alignment);
    void SetLineSpacing(float spacing);
    void SetGlyphSpacing(float spacing);

  protected:
    LayoutOptions() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<HAlignment> halignment_;
    Parser::EnumField<VAlignment> valignment_;
    Parser::TField<float>         line_spacing_;
    Parser::TField<float>         glyph_spacing_;
    Parser::TField<bool>          use_metrics_;
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
