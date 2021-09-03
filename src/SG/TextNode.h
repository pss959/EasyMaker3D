#pragma once

#include <string>

#include <ion/text/fontimage.h>
#include <ion/text/outlinebuilder.h>

#include "Math/Types.h"
#include "SG/LayoutOptions.h"
#include "SG/Node.h"
#include "SG/Typedefs.h"

namespace SG {

//! TextNode represents a text string to display. It is derived from Node
//! rather than Shape because the Ion text builder generates a Node (with
//! uniforms, etc.). However, it does not inherit any fields from Node.
class TextNode : public Node {
  public:
    virtual void AddFields() override;

    virtual void SetUpIon(IonContext &context) override;

    const std::string &    GetText()            const { return text_; }
    const std::string &    GetFontName()        const { return font_name_; }
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

    //! Updates the text string.
    void SetText(const std::string &new_text);

  private:
    //! \name Parsed Fields
    //!@{
    Parser::TField<std::string>        text_{"text"};
    Parser::TField<std::string>        font_name_{"font_name", "Arial"};
    Parser::TField<unsigned int>       font_size_{"font_size", 12U};
    Parser::TField<unsigned int>       sdf_padding_{"sdf_padding", 16U};
    Parser::TField<unsigned int>       max_image_size_{"max_image_size", 512U};
    Parser::TField<Vector4f>           color_{"color_", {1, 1, 1, 1}};
    Parser::TField<Vector4f>           outline_color_{"outline_color_", {0, 0, 0, 1}};
    Parser::TField<float>              outline_width_{"outline_width", 2.f};
    Parser::TField<float>              half_smooth_width_{"half_smooth_width", 3.f};
    Parser::ObjectField<LayoutOptions> layout_options_{"layout_options"};
    //!@}

    //! FontImage used for the text.
    ion::text::FontImagePtr font_image_;

    //! This is used to build or rebuild the Ion text.
    ion::text::OutlineBuilderPtr builder_;

    //! Returns an Ion FontImage to represent the TextNode's text. Uses a
    //! cached version if it already exists in the FontManager.
    ion::text::FontImagePtr GetFontImage_(IonContext &context) const;

    //! Builds or rebuilds the Ion text with the current data. Returns false on
    //! error.
    bool BuildText_();
};

}  // namespace SG
