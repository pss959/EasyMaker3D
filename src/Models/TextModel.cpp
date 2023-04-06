#include "Models/TextModel.h"

#include <vector>

#include "Math/MeshBuilding.h"
#include "Math/MeshCombining.h"
#include "Math/MeshUtils.h"
#include "Math/TextUtils.h"
#include "Util/Assert.h"
#include "Util/FilePath.h"
#include "Util/Tuning.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

namespace {

/// Scales each Polygon in the vector so that they are approximately the given
/// size in Y. Also translates all Polygons so that the entire set is centered
/// on the origin.
static void ScaleAndCenterPolygons_(std::vector<Polygon> &polygons,
                                    float y_size) {
    // Rectanglular range bounding all Polygons.
    Range2f rect;
    for (const Polygon &poly: polygons)
        rect.ExtendByRange(poly.GetBoundingRect());

    // Translate the Rect center to the origin and then scale.
    const float    scale = y_size / rect.GetSize()[1];
    const Vector2f trans = Vector2f(-rect.GetCenter());
    for (Polygon &poly: polygons) {
        poly.Translate(trans);
        poly.Scale(scale);
    }
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// TextModel functions.
// ----------------------------------------------------------------------------

void TextModel::AddFields() {
    AddModelField(text_.Init("text"));
    AddModelField(font_name_.Init("font_name",       TK::k3DFont));
    AddModelField(char_spacing_.Init("char_spacing", 1));

    PrimitiveModel::AddFields();
}

bool TextModel::IsValid(std::string &details) {
    if (! PrimitiveModel::IsValid(details))
        return false;

    if (font_name_.WasSet() && ! IsValidFontName(GetFontName())) {
        details = "Unknown font name";
        return false;
    }

    if (GetTextString().empty()) {
        details = "Empty text string";
        return false;
    }

    return true;
}

void TextModel::SetTextString(const std::string &text) {
    ASSERT(! text.empty());
    ValidateText_(font_name_, text);
    text_ = text;
    ProcessChange(SG::Change::kGeometry, *this);
}

void TextModel::SetFontName(const std::string &name) {
    ASSERT(IsValidFontName(name));
    ValidateText_(name, text_);
    font_name_ = name;
    ProcessChange(SG::Change::kGeometry, *this);
}

void TextModel::SetCharSpacing(float spacing) {
    char_spacing_ = spacing;
    ProcessChange(SG::Change::kGeometry, *this);
}

TriMesh TextModel::BuildMesh() {
    std::vector<Polygon> polygons =
        GetTextOutlines(GetFontName(), GetTextString(),
                        GetComplexity(), GetCharSpacing());

    // Scale and center all the 2D polygons so that a single line of text is
    // approximately kCharYSize units in Y and the entire text block is
    // centered on the origin.
    ScaleAndCenterPolygons_(polygons, TK::kTextHeight);

    // Extrude each polygon and combine the results.
    auto extrude = [](const Polygon &p){
        return BuildExtrudedMesh(p, TK::kExtrudedTextLength);
    };

    return CenterMesh(
        CombineMeshes(
            Util::ConvertVector<TriMesh, Polygon>(polygons, extrude),
            MeshCombiningOperation::kConcatenate));
}

void TextModel::ValidateText_(const std::string &font_name,
                              const std::string &text) {
    std::string reason;
    ASSERTM(IsValidStringForFont(font_name, text, reason), reason);
}
