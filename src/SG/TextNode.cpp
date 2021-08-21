#include "SG/TextNode.h"

#include <assert.h>

#include "SG/Exception.h"
#include "SG/LayoutOptions.h"
#include "SG/SpecBuilder.h"
#include "SG/Tracker.h"
#include "Util/Read.h"

namespace SG {

void TextNode::SetUpIon(IonContext &context) {
    if (! GetIonNode()) {
        // XXXX
        // XXXX Call SetIonNode().
    }
}

Parser::ObjectSpec TextNode::GetObjectSpec() {
    SG::SpecBuilder<TextNode> builder;
    builder.AddString("text",                  &TextNode::text_);
    builder.AddString("font_name",             &TextNode::font_name_);
    builder.AddString("font_path",             &TextNode::font_path_);
    builder.AddUInt("font_size",               &TextNode::font_size_);
    builder.AddUInt("sdf_padding",             &TextNode::sdf_padding_);
    builder.AddUInt("max_image_size",          &TextNode::max_image_size_);
    builder.AddVector4f("color",               &TextNode::color_);
    builder.AddVector4f("outline_color",       &TextNode::outline_color_);
    builder.AddFloat("outline_width",          &TextNode::outline_width_);
    builder.AddFloat("half_smooth_width",      &TextNode::half_smooth_width_);
    builder.AddObject<LayoutOptions>("layout", &TextNode::layout_options_);
    return Parser::ObjectSpec{
        "TextNode", false, []{ return new TextNode; }, builder.GetSpecs() };
}

}  // namespace SG
