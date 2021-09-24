#pragma once

#include <string>

#include <ion/gfxutils/shadermanager.h>
#include <ion/text/fontimage.h>
#include <ion/text/fontmanager.h>
#include <ion/text/outlinebuilder.h>

#include "Math/Types.h"
#include "SG/LayoutOptions.h"
#include "SG/Node.h"
#include "SG/Typedefs.h"

namespace SG {

/// TextNode represents a text string to display. It is derived from Node
/// rather than Shape because the Ion text builder generates a Node (with
/// uniforms, etc.). However, it does not inherit any fields from Node.
class TextNode : public Node {
  public:
    virtual void AddFields() override;

    /// If not already done, this creates and adds Ion text to the Ion Node
    /// attached to this TextNode, which must already exist.
    void AddIonText(ion::text::FontManager &font_manager,
                    ion::gfxutils::ShaderManagerPtr &shader_manager);

    const std::string & GetText()            const { return text_; }
    const std::string & GetFontName()        const { return font_name_; }
    unsigned int        GetFontSize()        const { return font_size_; }
    unsigned int        GetSDFPadding()      const { return sdf_padding_; }
    unsigned int        GetMaxImageSize()    const { return max_image_size_; }
    const Color &       GetColor()           const { return color_; }
    const Color &       GetOutlineColor()    const { return outline_color_; }
    float               GetOutlineWidth()    const { return outline_width_; }
    float               GetHalfSmoothWidth() const {
        return half_smooth_width_;
    }
    const LayoutOptionsPtr GetLayoutOptions()   const { return layout_options_; }

    /// Updates the text string.
    void SetText(const std::string &new_text);

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string>  text_{"text"};
    Parser::TField<std::string>  font_name_{"font_name", "Arial"};
    Parser::TField<unsigned int> font_size_{"font_size", 12U};
    Parser::TField<unsigned int> sdf_padding_{"sdf_padding", 16U};
    Parser::TField<unsigned int> max_image_size_{"max_image_size", 512U};
    Parser::TField<Color>        color_{"color", Color::White()};
    Parser::TField<Color>        outline_color_{"outline_color",
                                                Color::Black()};
    Parser::TField<float>        outline_width_{"outline_width", 2.f};
    Parser::TField<float>        half_smooth_width_{"half_smooth_width", 3.f};
    Parser::ObjectField<LayoutOptions> layout_options_{"layout"};
    ///@}

    /// FontImage used for the text.
    ion::text::FontImagePtr font_image_;

    /// This is used to build or rebuild the Ion text.
    ion::text::OutlineBuilderPtr builder_;

    /// Returns an Ion FontImage to represent the TextNode's text. Uses a
    /// cached version if it already exists in the FontManager.
    ion::text::FontImagePtr GetFontImage_(
        ion::text::FontManager &font_manager) const;

    /// Builds or rebuilds the Ion text with the current data. Returns false on
    /// error.
    bool BuildText_();
};

}  // namespace SG
