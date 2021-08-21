#pragma once

#include <string>

#include "Parser/ObjectSpec.h"
#include "SG/Math.h"
#include "SG/Node.h"
#include "SG/Typedefs.h"

namespace SG {

//! TextNode represents a text string to display. It is derived from Node
//! rather than Shape because the Ion text builder generates a Node (with
//! uniforms, etc.). However, it does not inherit any fields from Node.
class TextNode : public Node {
  public:
    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

    const std::string &    GetText()            const { return text_; }
    const std::string &    GetFontName()        const { return font_name_; }
    const std::string &    GetFontPath()        const { return font_path_; }
    unsigned int           GetFontSize()        const { return font_size_; }
    unsigned int           GetSDFPadding()      const { return sdf_padding_; }
    unsigned int           GetMaxImageSize()    const { return max_image_size_; }
    const Vector4f &       GetColor()           const { return color_; }
    const Vector4f &       GetOutlineColor()    const { return outline_color_; }
    float                  GetOutlineWidth()    const { return outline_width_; }
    float                  GetHalfSmoothWidth() const {
        return half_smooth_width_;
    }
    const LayoutOptionsPtr GetLayoutOptions()   const { return layout_options_; }

  private:
    //! \name Parsed Fields
    //!@{
    std::string      text_;
    std::string      font_name_;
    std::string      font_path_;
    unsigned int     font_size_      = 0;
    unsigned int     sdf_padding_    = 0;
    unsigned int     max_image_size_ = 0;
    Vector4f         color_             = Vector4f(1.f, 1.f, 1.f, 1.f);
    Vector4f         outline_color_     = Vector4f::Zero();
    float            outline_width_     = 2.f;
    float            half_smooth_width_ = 3.f;
    LayoutOptionsPtr layout_options_;
    //!@}
};

}  // namespace SG
