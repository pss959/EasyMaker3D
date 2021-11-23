#pragma once

#include <ion/text/layout.h>

#include "Math/Types.h"
#include "SG/Object.h"

namespace Parser { class Registry; }

namespace SG {

/// A LayoutOptions object wraps an Ion LayoutOptions struct. Note that the
/// target_size and target_point values are not settable, since the TextNode
/// expects them to have default settings.
class LayoutOptions : public Object {
  public:
    typedef ion::text::HorizontalAlignment HAlignment;
    typedef ion::text::VerticalAlignment   VAlignment;

    virtual void AddFields() override;

    HAlignment       GetHAlignment()    const { return halignment_;    }
    VAlignment       GetVAlignment()    const { return valignment_;    }
    float            GetLineSpacing()   const { return line_spacing_;  }
    float            GetGlyphSpacing()  const { return glyph_spacing_; }
    bool IsUsingMetricsBasedAlignment() const { return use_metrics_;   }

    void SetHAlignment(HAlignment alignment);
    void SetVAlignment(VAlignment alignment);

  protected:
    LayoutOptions() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<HAlignment> halignment_{
        "halignment", HAlignment::kAlignLeft};
    Parser::EnumField<VAlignment> valignment_{
        "valignment", VAlignment::kAlignBaseline};
    Parser::TField<float>         line_spacing_{"line_spacing", 1.f};
    Parser::TField<float>         glyph_spacing_{"glyph_spacing", 0.f};
    Parser::TField<bool>          use_metrics_{"use_metrics", false};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
