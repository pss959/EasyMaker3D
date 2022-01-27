#include "Panels/InfoPanel.h"

#include "Math/Linear.h"
#include "Models/Model.h"
#include "Targets/EdgeTarget.h"
#include "Targets/PointTarget.h"
#include "Util/Assert.h"
#include "Util/String.h"

void InfoPanel::CreationDone() {
    Panel::CreationDone();

    auto &root_pane = GetPane();
    contents_pane_ = root_pane->FindTypedPane<ContainerPane>("Contents");
    text_pane_     = root_pane->FindTypedPane<TextPane>("InfoText");
}

void InfoPanel::SetInfo(const Info &info) {
    ASSERT(info.selection.HasAny() || info.point_target || info.edge_target);

    // Create a vector containing a clone of the TextPane for each line of
    // information.
    std::vector<PanePtr> panes;

    for (const auto &sel_path: info.selection.GetPaths())
        AddModelInfo_(panes, sel_path);

    // XXXX Add target stuff.

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
    ASSERT(sel_path.GetModel());
    const Model &model = *sel_path.GetModel();

    AddTextPane_(panes, TextType_::kHeader,
                 "Model \"" + model.GetName() + "\"");

    const TriMesh &mesh = model.GetMesh();
    std::string reason;
    if (! model.IsMeshValid(reason))
        AddTextPane_(panes, TextType_::kError, "Invalid mesh: " + reason);
    AddTextPane_(panes, TextType_::kNormal,
                 "Vertex Count: " + Util::ToString(mesh.points.size()));
    AddTextPane_(panes, TextType_::kNormal,
                 "Triangle Count: " + Util::ToString(mesh.GetTriangleCount()));

    const Matrix4f osm = sel_path.GetObjectToStageMatrix();
    const Vector3f size = TransformBounds(model.GetBounds(), osm).GetSize();
    AddTextPane_(panes, TextType_::kNormal,
                 "Width: " + Util::ToString(size[0]));
    AddTextPane_(panes, TextType_::kNormal,
                 "Depth: " + Util::ToString(size[2]));
    AddTextPane_(panes, TextType_::kNormal,
                 "Height " + Util::ToString(size[1]));
}

void InfoPanel::AddTextPane_(std::vector<PanePtr> &panes, TextType_ type,
                             const std::string &text) {
    const std::string name = "Line" + Util::ToString(panes.size());

    std::string str;
    Color       color;

    switch (type) {
      case TextType_::kHeader:
        color = Color(.2f, .2f, 1);
        str   = text;
        break;
      case TextType_::kError:
        color = Color(1, .2f, .2f);
        str   = "    " + text;
        break;
      case TextType_::kNormal:
        color = Color::Black();
        str   = "    " + text;
        break;
    }

    auto pane = text_pane_->CloneTyped<TextPane>(true, name);
    pane->SetText(str);
    pane->SetColor(color);
    pane->SetEnabled(SG::Node::Flag::kTraversal, true);
    panes.push_back(pane);
}
