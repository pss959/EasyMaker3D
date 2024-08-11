//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Panels/InfoPanel.h"

#include "Math/Linear.h"
#include "Math/ToString.h"
#include "Models/Model.h"
#include "Panes/ContainerPane.h"
#include "Panes/TextPane.h"
#include "Place/EdgeTarget.h"
#include "Place/PointTarget.h"
#include "SG/ColorMap.h"
#include "SG/CoordConv.h"
#include "Util/Assert.h"
#include "Util/String.h"
#include "Util/Tuning.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

namespace {

/// \name Conversion functions,
/// Each of these converts a value of some type to a printable string.
///@{

static Str ToString_(float f) {
    return Math::ToString(f, .01f);
}
static Str ToString_(const Point3f &p) {
    return Math::ToString(ToPrintCoords(p), .01f);
}
static Str ToString_(const Vector3f &v) {
    return Math::ToString(ToPrintCoords(v), .01f);
}
static Str ToString_(const Anglef &a) {
    return Math::ToString(a.Degrees(), .01f);
}

///@}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// InfoPanel functions.
// ----------------------------------------------------------------------------

void InfoPanel::SetInfo(const Info &info) {
    ASSERT(info.selection.HasAny() || info.point_target || info.edge_target);

    // Create a vector containing a clone of the TextPane for each line of
    // information.
    Pane::PaneVec panes;

    for (const auto &sel_path: info.selection.GetPaths())
        AddModelInfo_(panes, sel_path);

    if (info.point_target)
        AddPointTargetInfo_(panes, *info.point_target);
    if (info.edge_target)
        AddEdgeTargetInfo_(panes, *info.edge_target);

    contents_pane_->ReplacePanes(panes);
}

void InfoPanel::InitInterface() {
    auto &root_pane = GetPane();
    contents_pane_  = root_pane->FindTypedSubPane<ContainerPane>("Contents");
    text_pane_      = root_pane->FindTypedSubPane<TextPane>("InfoText");
    separator_pane_ = root_pane->FindSubPane("InfoSeparator");

    AddButtonFunc("Done", [this](){ Close("Done");     });
}

void InfoPanel::UpdateInterface() {
    SetFocus("Done");
}

void InfoPanel::AddModelInfo_(Pane::PaneVec &panes, const SelPath &sel_path) {
    AddSeparator_(panes);

    ASSERT(sel_path.GetModel());
    const Model &model = *sel_path.GetModel();

    AddTextPane_(panes, TextType_::kHeader,
                 "", "Model \"" + model.GetName() + "\"");

    const TriMesh &mesh = model.GetMesh();
    Str reason;
    const bool is_valid = model.IsMeshValid(reason);
    if (! is_valid)
        AddTextPane_(panes, TextType_::kError, "Invalid mesh", reason);
    AddTextPane_(panes, TextType_::kNormal,
                 "Vertex Count",  Util::ToString(mesh.points.size()));
    AddTextPane_(panes, TextType_::kNormal,
                 "Triangle Count", Util::ToString(mesh.GetTriangleCount()));

    const Matrix4f osm    = SG::CoordConv(sel_path).GetObjectToRootMatrix();
    const Bounds   bounds = TransformBounds(model.GetBounds(), osm);
    const Point3f  center = bounds.GetCenter();
    const Vector3f size   = bounds.GetSize();
    AddTextPane_(panes, TextType_::kNormal, "Width",  ToString_(size[0]));
    AddTextPane_(panes, TextType_::kNormal, "Depth",  ToString_(size[2]));
    AddTextPane_(panes, TextType_::kNormal, "Height", ToString_(size[1]));
    AddTextPane_(panes, TextType_::kNormal, "Center",
                 ToString_(center[0]) + " " + ToString_(center[2]));
    // Cannot compute volume for an invalid mesh.
    if (is_valid)
        AddTextPane_(panes, TextType_::kNormal, "Volume",
                     ToString_(model.ComputeVolume()));
}

void InfoPanel::AddPointTargetInfo_(Pane::PaneVec &panes,
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

void InfoPanel::AddEdgeTargetInfo_(Pane::PaneVec &panes, const EdgeTarget &et) {
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

void InfoPanel::AddTextPane_(Pane::PaneVec &panes, TextType_ type,
                             const Str &label, const Str &text) {
    // Labels use this many characters.
    Str sized_label = label;
    if (! sized_label.empty()) {
        sized_label += ":";
        sized_label.append(TK::kInfoPanelLabelWidth - sized_label.size(), ' ');
    }

    const Str name = "Line" + Util::ToString(panes.size());

    Str      font_name;
    Color    color;
    Vector2f offset(0, 0);  // Offset in world coords, not Panel coords.

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

void InfoPanel::AddSeparator_(Pane::PaneVec &panes) {
    if (! panes.empty()) {
        auto sep = separator_pane_->CloneTyped<Pane>(true);
        sep->SetEnabled(true);
        panes.push_back(sep);
    }
}
