//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <ion/gfxutils/shadermanager.h>
#include <ion/text/fontimage.h>
#include <ion/text/fontmanager.h>
#include <ion/text/outlinebuilder.h>

#include "Math/Types.h"
#include "SG/LayoutOptions.h"
#include "SG/Node.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(TextNode);

/// TextNode represents a text string to display. It is derived from Node
/// rather than Shape because the Ion text builder generates a Node (with
/// uniforms, etc.). However, it does not inherit any fields from Node.
///
/// Note that this always leaves the text at its default size, which is the
/// size of the text when ion::text::LayoutOptions target_size is left at its
/// default value of (0,1). The same is true for position, which is defined by
/// a target_point of (0,0). The scale and translation in the TextNode can be
/// used to resize or move text as needed.
///
/// \ingroup SG
class TextNode : public Node {
  public:
    ~TextNode();

    const Str &   GetText()            const { return text_; }
    const Str &   GetFontName()        const { return font_name_; }
    unsigned int  GetFontSize()        const { return font_size_; }
    unsigned int  GetSDFPadding()      const { return sdf_padding_; }
    unsigned int  GetMaxImageSize()    const { return max_image_size_; }
    const Color & GetColor()           const { return color_; }
    const Color & GetOutlineColor()    const { return outline_color_; }
    float         GetOutlineWidth()    const { return outline_width_; }
    float         GetHalfSmoothWidth() const { return half_smooth_width_; }
    const LayoutOptionsPtr GetLayoutOptions() const { return layout_options_; }

    void SetFontName(const Str &font_name);
    void SetFontSize(unsigned int font_size);

    /// Updates the text string.
    void SetText(const Str &new_text);

    /// Updates the text color.
    void SetTextColor(const Color &color);

    /// Updates the text string and sets it to the given color.
    void SetTextWithColor(const Str &new_text, const Color &color);

    /// Sets the LayoutOptions.
    void SetLayoutOptions(const LayoutOptionsPtr &opts);

    /// Returns the height factor that line spacing contributes to multi-line
    /// text. That is, the line spacing should be multiplied by this factor to
    /// determine how far apart the baselines are for two lines of text. This
    /// assumes that SetUpIon() was called.
    float GetLineSpacingFactor() const;

    /// Returns the text bounds (without scale and translation applied). These
    /// will be empty until the text is built the first time (in SetUpIon()).
    const Bounds & GetTextBounds();

    /// Returns the size of the text. This will be (0,0) until the text is
    /// built the first time (in SetUpIon()).
    const Vector2f & GetTextSize();

    /// Redefines this to also create and adds Ion text to the Ion Node.
    virtual ion::gfx::NodePtr SetUpIon(
        const IonContextPtr &ion_context,
        const std::vector<ion::gfx::ShaderProgramPtr> &programs) override;

    /// Redefines this to also rebuild the text if necessary.
    virtual void UpdateForRenderPass(const Str &pass_name) override;

    /// Sets the scale and rotation fields so the text faces the positive Z
    /// axis and is a reasonable size. This is passed a matrix used to convert
    /// from object to world coordinates, a base scale factor, and the current
    /// object-space rotation.
    void SetWorldScaleAndRotation(const Matrix4f &owm, float base_scale,
                                  const Rotationf &current_rotation);

  protected:
    TextNode() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

    /// Redefines this to use the bounds of the text rectangle.
    virtual Bounds UpdateBounds() const;

    /// Redefines this to rebuild text when LayoutOptions change.
    virtual bool ProcessChange(Change change, const Object &obj) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Str>                text_;
    Parser::TField<Str>                font_name_;
    Parser::TField<unsigned int>       font_size_;
    Parser::TField<unsigned int>       sdf_padding_;
    Parser::TField<unsigned int>       max_image_size_;
    Parser::TField<Color>              color_;
    Parser::TField<Color>              outline_color_;
    Parser::TField<float>              outline_width_;
    Parser::TField<float>              half_smooth_width_;
    Parser::ObjectField<LayoutOptions> layout_options_;
    ///@}

    /// FontImage used for the text.
    ion::text::FontImagePtr      font_image_;

    /// This is used to build or rebuild the Ion text.
    ion::text::OutlineBuilderPtr builder_;

    /// Saves the bounds of the built text.
    Bounds                       text_bounds_;

    /// Saves the size of the built text.
    Vector2f                     text_size_{0, 0};

    /// Flag indicating whether the text needs to be rebuilt.
    bool                         needs_rebuild_ = true;

    void SetUpFont_();

    /// Builds or rebuilds the Ion text with the current data. Returns false on
    /// error.
    bool BuildText_();

    /// Returns an Ion FontImage to represent the TextNode's text. Uses a
    /// cached version if it already exists in the FontManager.
    ion::text::FontImagePtr GetFontImage_(
        ion::text::FontManager &font_manager) const;

    friend class Parser::Registry;
};

}  // namespace SG
