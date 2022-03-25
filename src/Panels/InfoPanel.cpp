#include "Panels/InfoPanel.h"

#include "CoordConv.h"
#include "Math/Linear.h"
#include "Models/Model.h"
#include "Panes/ContainerPane.h"
#include "Panes/TextPane.h"
#include "Targets/EdgeTarget.h"
#include "Targets/PointTarget.h"
#include "Util/Assert.h"
#include "Util/String.h"

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

    const Matrix4f osm = CoordConv(sel_path).GetObjectToRootMatrix();
    const Vector3f size = TransformBounds(model.GetBounds(), osm).GetSize();
    AddTextPane_(panes, TextType_::kNormal, "Width",  Util::ToString(size[0]));
    AddTextPane_(panes, TextType_::kNormal, "Depth",  Util::ToString(size[2]));
    AddTextPane_(panes, TextType_::kNormal, "Height", Util::ToString(size[1]));
}

void InfoPanel::AddPointTargetInfo_(std::vector<PanePtr> &panes,
                                    const PointTarget &pt) {
    AddSeparator_(panes);

    AddTextPane_(panes, TextType_::kHeader, "", "Point Target");
    AddTextPane_(panes, TextType_::kNormal,
                 "Position",  Util::ToString(pt.GetPosition(), .01f));
    AddTextPane_(panes, TextType_::kNormal,
                 "Direction", Util::ToString(pt.GetDirection(), .01f));
    AddTextPane_(panes, TextType_::kNormal,
                 "Radius",    Util::ToString(pt.GetRadius(), .01f));
    AddTextPane_(panes, TextType_::kNormal,
                 "Start Angle",
                 Util::ToString(pt.GetStartAngle().Degrees(), .01f));
    AddTextPane_(panes, TextType_::kNormal,
                 "Arc Angle", Util::ToString(pt.GetArcAngle().Degrees(), .01f));
}

void InfoPanel::AddEdgeTargetInfo_(std::vector<PanePtr> &panes,
                                   const EdgeTarget &et) {
    AddSeparator_(panes);

    AddTextPane_(panes, TextType_::kHeader, "", "Edge Target");
    AddTextPane_(panes, TextType_::kNormal,
                 "Position 0", Util::ToString(et.GetPosition0(), .01f));
    AddTextPane_(panes, TextType_::kNormal,
                 "Position 1", Util::ToString(et.GetPosition1(), .01f));
    AddTextPane_(panes, TextType_::kNormal,
                 "Direction",  Util::ToString(et.GetDirection(), .01f));
    AddTextPane_(panes, TextType_::kNormal,
                 "Length",     Util::ToString(et.GetLength(), .01f));
}

void InfoPanel::AddTextPane_(std::vector<PanePtr> &panes, TextType_ type,
                             const std::string &label,
                             const std::string &text) {
    // Labels use this many characters.
    const size_t kLabelWidth = 16;
    std::string sized_label = label;
    if (! sized_label.empty()) {
        sized_label += ":";
        sized_label.append(kLabelWidth - sized_label.length(), ' ');
    }

    const std::string name = "Line" + Util::ToString(panes.size());

    std::string font_name;
    Color       color;
    Vector2f    offset(0, 0);  // Offset in world coords, not Panel coords.

    switch (type) {
      case TextType_::kHeader:
        font_name = "Verdana";
        color     = Color(.2f, .2f, 1);
        break;
      case TextType_::kError:
        font_name = "Verdana_Italic";
        color = Color(1, .2f, .2f);
        offset[0] = .04f;
        break;
      case TextType_::kNormal:
        font_name = "Mono";
        color = Color::Black();
        offset[0] = .04f;
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
