#pragma once

#include <ion/text/layout.h>

#include "Math/Types.h"
#include "Parser/ObjectSpec.h"
#include "SG/Object.h"

namespace SG {

//! A LayoutOptions object wraps an Ion LayoutOptions struct.
class LayoutOptions : public Object {
  public:
    typedef ion::text::HorizontalAlignment HAlignment;
    typedef ion::text::VerticalAlignment   VAlignment;

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

    static Parser::ObjectSpec GetObjectSpec();

  private:
    //! Associated Ion LayoutOptions.
    ion::text::LayoutOptions  ion_layout_options_;

    //! \name Parsed Fields
    //!@{
    Point2f    target_point_{ 0, 0};
    Vector2f   target_size_{ 0, 1 };
    HAlignment halignment_    = HAlignment::kAlignLeft;
    VAlignment valignment_    = VAlignment::kAlignBaseline;
    float      line_spacing_  = 1.f;
    float      glyph_spacing_ = 0.f;
    bool       use_metrics_   = false;
    //!@}
};

}  // namespace SG
