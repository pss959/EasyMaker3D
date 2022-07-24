#pragma once

#include <ion/text/layout.h>

#include "Base/Memory.h"
#include "Math/Types.h"
#include "SG/Object.h"

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
    typedef ion::text::HorizontalAlignment HAlignment;
    typedef ion::text::VerticalAlignment   VAlignment;

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
