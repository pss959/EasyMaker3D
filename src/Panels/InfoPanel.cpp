#include "Panels/InfoPanel.h"

#include "App/CoordConv.h"
#include "Base/Tuning.h"
#include "Math/Linear.h"
#include "Math/ToString.h"
#include "Models/Model.h"
#include "Panes/ContainerPane.h"
#include "Panes/TextPane.h"
#include "SG/ColorMap.h"
#include "Targets/EdgeTarget.h"
#include "Targets/PointTarget.h"
#include "Util/Assert.h"
#include "Util/String.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

namespace {

/// \name Conversion functions,
/// Each of these converts a value of some type to a printable string.
///@{

static std::string ToString_(float f) {
    return Math::ToString(f, .01f);
}
static std::string ToString_(const Point3f &p) {
    return Math::ToString(ToPrintCoords(p), .01f);
}
static std::string ToString_(const Vector3f &v) {
    return Math::ToString(ToPrintCoords(v), .01f);
}
static std::string ToString_(const Anglef &a) {
    return Math::ToString(a.Degrees(), .01f);
}

///@}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// InfoPanel functions.
// ----------------------------------------------------------------------------

void InfoPanel::CreationDone() {
    Panel::CreationDone();

    auto &root_pane = GetPane();
    contents_pane_  = root_pane->FindTypedPane<ContainerPane>("Contents");
    text_pane_      = root_pane->FindTypedPane<TextPane>("InfoText");
    separator_pane_ = root_pane->FindPane("InfoSeparator");
}

void InfoPanel::SetInfo(const Info &info) {
    ASSERT(info.selection.HasAny() || info.point_target || info.edge_target);

    // Create a vector containing a clone of the TextPane for each line of
    // information.
    std::vector<PanePtr> panes;

    for (const auto &sel_path: info.selection.GetPaths())
        AddModelInfo_(panes, sel_path);

    if (info.point_target)
        AddPointTargetInfo_(panes, *info.point_target);
    if (info.edge_target)
        AddEdgeTargetInfo_(panes, *info.edge_target);

    contents_pane_->ReplacePanes(panes);
}

void InfoPanel::InitInterface() {
    AddButtonFunc("Done", [this](){ Close("Done");     });
}

void InfoPanel::UpdateInterface() {
    SetFocus("Done");
}

void InfoPanel::AddModelInfo_(std::vector<PanePtr> &panes,
                              const SelPath &sel_path) {
    AddSeparator_(panes);

    ASSERT(sel_path.GetModel());
    const Model &model = *sel_path.GetModel();

    AddTextPane_(panes, TextType_::kHeader,
                 "", "Model \"" + model.GetName() + "\"");

    const TriMesh &mesh = model.GetMesh();
    std::string reason;
    if (! model.IsMeshValid(reason))
        AddTextPane_(panes, TextType_::kError, "Invalid mesh", reason);
    AddTextPane_(panes, TextType_::kNormal,
                 "Vertex Count",  Util::ToString(mesh.points.size()));
    AddTextPane_(panes, TextType_::kNormal,
                 "Triangle Count", Util::ToString(mesh.GetTriangleCount()));

    const Matrix4f osm    = CoordConv(sel_path).GetObjectToRootMatrix();
    const Bounds   bounds = TransformBounds(model.GetBounds(), osm);
    const Point3f  center = bounds.GetCenter();
    const Vector3f size   = bounds.GetSize();
    AddTextPane_(panes, TextType_::kNormal, "Width",  ToString_(size[0]));
    AddTextPane_(panes, TextType_::kNormal, "Depth",  ToString_(size[2]));
    AddTextPane_(panes, TextType_::kNormal, "Height", ToString_(size[1]));
    AddTextPane_(panes, TextType_::kNormal, "Center",
                 ToString_(center[0]) + " " + ToString_(center[2]));
}

void InfoPanel::AddPointTargetInfo_(std::vector<PanePtr> &panes,
                                    const PointTarget &pt) {
    AddSeparator_(panes);

    AddTextPane_(panes, TextType_::kHeader, "", "Point Target");
    AddTextPane_(panes, TextType_::kNormal,
                 "Position",    ToString_(pt.GetPosition()));
    AddTextPane_(panes, TextType_::kNormal,
                 "Direction",   ToString_(pt.GetDirection()));
    AddTextPane_(panes, TextType_::kNormal,
                 "Radius",      ToString_(pt.GetRadius()));
    AddTextPane_(panes, TextType_::kNormal,
                 "Start Angle", ToString_(pt.GetArc().start_angle));
    AddTextPane_(panes, TextType_::kNormal,
                 "Arc Angle",   ToString_(pt.GetArc().arc_angle));
}

void InfoPanel::AddEdgeTargetInfo_(std::vector<PanePtr> &panes,
                                   const EdgeTarget &et) {
    AddSeparator_(panes);

    AddTextPane_(panes, TextType_::kHeader, "", "Edge Target");
    AddTextPane_(panes, TextType_::kNormal,
                 "Position 0", ToString_(et.GetPosition0()));
    AddTextPane_(panes, TextType_::kNormal,
                 "Position 1", ToString_(et.GetPosition1()));
    AddTextPane_(panes, TextType_::kNormal,
                 "Direction",  ToString_(et.GetDirection()));
    AddTextPane_(panes, TextType_::kNormal,
                 "Length",     ToString_(et.GetLength()));
}

void InfoPanel::AddTextPane_(std::vector<PanePtr> &panes, TextType_ type,
                             const std::string &label,
                             const std::string &text) {
    // Labels use this many characters.
    std::string sized_label = label;
    if (! sized_label.empty()) {
        sized_label += ":";
        sized_label.append(TK::kInfoPanelLabelWidth - sized_label.size(), ' ');
    }

    const std::string name = "Line" + Util::ToString(panes.size());

    std::string font_name;
    Color       color;
    Vector2f    offset(0, 0);  // Offset in world coords, not Panel coords.

    switch (type) {
      case TextType_::kHeader:
        font_name = TK::kInfoPanelHeaderFont;
        color     = SG::ColorMap::SGetColor("InfoPanelHeaderColor");
        break;
      case TextType_::kError:
        font_name = TK::kInfoPanelErrorFont;
        color     = SG::ColorMap::SGetColor("InfoPanelErrorColor");
        offset[0] = TK::kInfoPanelIndent;
        break;
      case TextType_::kNormal:
        font_name = TK::kInfoPanelRegularFont;
        color     = SG::ColorMap::SGetColor("InfoPanelRegularColor");
        offset[0] = TK::kInfoPanelIndent;
        break;
    }

    auto pane = text_pane_->CloneTyped<TextPane>(true, name);
    pane->SetFontName(font_name);
    pane->SetText(sized_label + text);
    pane->SetColor(color);
    pane->SetOffset(offset);
    pane->SetEnabled(true);
    panes.push_back(pane);
}

void InfoPanel::AddSeparator_(std::vector<PanePtr> &panes) {
    if (! panes.empty()) {
        auto sep = separator_pane_->CloneTyped<Pane>(true);
        sep->SetEnabled(true);
        panes.push_back(sep);
    }
}
