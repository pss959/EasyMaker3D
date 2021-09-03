#pragma once

#include <ion/text/layout.h>

#include "Math/Types.h"
#include "SG/Object.h"

namespace SG {

//! A LayoutOptions object wraps an Ion LayoutOptions struct.
class LayoutOptions : public Object {
  public:
    typedef ion::text::HorizontalAlignment HAlignment;
    typedef ion::text::VerticalAlignment   VAlignment;

    virtual void AddFields() override;

    //! Returns the associated Ion LayoutOptions. Note that because this is a
    //! struct, this returns a reference, not a smart pointer.
    const ion::text::LayoutOptions & GetIonLayoutOptions() const {
        return ion_layout_options_;
    }

    const Point2f  & GetTargetPoint()   const { return target_point_;  }
    const Vector2f & GetTargetSize()    const { return target_size_;   }
    HAlignment       GetHAlignment()    const { return halignment_;    }
    VAlignment       GetVAlignment()    const { return valignment_;    }
    float            GetLineSpacing()   const { return line_spacing_;  }
    float            GetGlyphSpacing()  const { return glyph_spacing_; }
    bool IsUsingMetricsBasedAlignment() const { return use_metrics_;   }

    virtual void SetUpIon(IonContext &context) override;

  private:
    //! Associated Ion LayoutOptions.
    ion::text::LayoutOptions  ion_layout_options_;

    //! \name Parsed Fields
    //!@{
    Parser::TField<Point2f>       target_point_{"target_point", {0, 0}};
    Parser::TField<Vector2f>      target_size_{"target_size", {0, 1}};
    Parser::EnumField<HAlignment> halignment_{
        "halignment", HAlignment::kAlignLeft};
    Parser::EnumField<VAlignment> valignment_{
        "valignment", VAlignment::kAlignBaseline};
    Parser::TField<float>         line_spacing_{"line_spacing", 1.f};
    Parser::TField<float>         glyph_spacing_{"glyph_spacing", 0.f};
    Parser::TField<bool>          use_metrics_{"use_metrics", false};
    //!@}
};

}  // namespace SG
