//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Debug/Print.h"

#if ENABLE_DEBUG_FEATURES

#include <iostream>
#include <stack>
#include <unordered_set>

#include <ion/math/transformutils.h>

#include "Commands/CommandList.h"
#include "Math/Frustum.h"
#include "Math/Linear.h"
#include "Math/MeshUtils.h"
#include "Math/ToString.h"
#include "Models/ParentModel.h"
#include "Panels/Board.h"
#include "Panels/Panel.h"
#include "Panes/DropdownPane.h"
#include "Panes/Pane.h"
#include "Panes/ScrollingPane.h"
#include "Parser/Writer.h"
#include "SG/Gantry.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/Shape.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"

namespace {

// ----------------------------------------------------------------------------
// Helper types and classes.
// ----------------------------------------------------------------------------

/// The PathLimiter_ class is used to track a path to limit printing to. It
/// returns true if the given Node should be printed, meaning it is either on
/// the path or below the last Node in it if print_below is true.
class PathLimiter_ {
  public:
    explicit PathLimiter_(const SG::NodePath &path, bool print_below) :
        path_(path),
        print_below_(print_below) {}

    /// Returns true if the given Node should be printed.
    bool Push(const SG::Node &node);
    void Pop();

  private:
    const SG::NodePath          path_;
    const bool                  print_below_;
    bool                        is_under_path_ = false;
    std::stack<const SG::Node*> nodes_;

    bool IsInPath_(const SG::Node &node) {
        for (const auto &path_node: path_)
            if (path_node.get() == &node)
                return true;
        return false;
    }
};

bool PathLimiter_::Push(const SG::Node &node) {
    bool add_node = false;

    if (IsInPath_(node)) {
        // If this is the tail of the path, set the flag for future nodes.
        if (&node == path_.back().get())
            is_under_path_ = true;
        add_node = true;
    }
    // A node not on the path is printed only if it is under the tail node and
    // print_below_ is set.
    else if (is_under_path_ && print_below_) {
        add_node = true;
    }
    if (add_node)
        nodes_.push(&node);
    return add_node;
}

void PathLimiter_::Pop() {
    if (is_under_path_ && nodes_.top() == path_.back().get())
        is_under_path_ = false;
    nodes_.pop();
}

/// The Surrounder_ class surrounds scoped text with horizontal lines.
class Surrounder_ {
  public:
    Surrounder_()  { PrintLines_(); }
    ~Surrounder_() { PrintLines_(); }
  private:
    static void PrintLines_() {
        std::cout << "--------------------------------------------------\n";
    }
};

// ----------------------------------------------------------------------------
// Generic helper functions.
// ----------------------------------------------------------------------------

static Str Indent_(int level, bool add_horiz = true) {
    Str s;
    for (int i = 1; i < level; ++i)
        s += "| ";
    if (level >= 1)
        s += add_horiz ? "|-" : "| ";
    return s;
}

static Str GetDesc_(const SG::Object &obj) {
    // Start with regular description string.
    Str s = obj.GetDesc();

    // If a Node, add disabled flags.
    if (const SG::Node *node = dynamic_cast<const SG::Node *>(&obj)) {
        const auto flags = node->GetDisabledFlags();
        if (flags.HasAny())
            s += " [" + flags.ToString() + "]";
    }
    return s;
}

// ----------------------------------------------------------------------------
// Printing helper functions.
// ----------------------------------------------------------------------------

/// Prints an SG::NodePath, returning true if it is not empty.
static bool PrintPath_(const SG::NodePath &path) {
    if (path.empty()) {
        std::cout << "<EMPTY PATH>\n";
        return false;
    }
    else {
        std::cout << "PATH = '" << path.ToString() << "'\n";
        return true;
    }
}

static Matrix4f PrintBounds_(const SG::Node &node, const Matrix4f &wsm,
                             int level, const Matrix4f &start_matrix) {
    Str indent = Indent_(level);
    const Matrix4f ctm = start_matrix * node.GetModelMatrix();

    auto print_bounds = [indent, ctm, wsm](const SG::Object &obj,
                                           const Bounds &bounds,
                                           const Str &extra){
        const Bounds wbounds = TransformBounds(bounds,  ctm);
        const Bounds sbounds = TransformBounds(wbounds, wsm);
        std::cout << indent << extra << GetDesc_(obj) << "\n"
                  << indent << "    LOC: " <<  bounds << "\n"
                  << indent << "    STG: " << sbounds << "\n"
                  << indent << "    WLD: " << wbounds.ToString(false) << "\n"
                  << indent << "    WLD: " << wbounds.ToString(true)  << "\n";
    };

    print_bounds(node, node.GetBounds(), "");
    indent = Indent_(level + 1);

    for (const auto &shape: node.GetShapes())
        print_bounds(*shape, shape->GetBounds(), "--");

    return ctm;
}

static void PrintBoundsRecursive_(const SG::Node &node, const Matrix4f &wsm,
                                  int level, const Matrix4f &start_matrix) {
    const Matrix4f ctm = PrintBounds_(node, wsm, level, start_matrix);
    for (const auto &child: node.GetAllChildren())
        PrintBoundsRecursive_(*child, wsm, level + 1, ctm);
}

static Matrix4f PrintLocations_(const SG::Node &node, const Matrix4f &wsm,
                                int level, const Matrix4f &start_matrix) {
    Str indent = Indent_(level);
    const Matrix4f ctm = start_matrix * node.GetModelMatrix();

    const auto lloc = Point3f::Zero() + node.GetTranslation();
    const auto wloc = ctm * lloc;
    const auto sloc = wsm * wloc;
    std::cout << indent << GetDesc_(node) << "\n"
              << indent
              << " L: " << lloc << " S: " << sloc << " W:" << wloc << "\n";

    return ctm;
}

static void PrintLocationsRecursive_(const SG::Node &node, const Matrix4f &wsm,
                                  int level, const Matrix4f &start_matrix) {
    const Matrix4f ctm = PrintLocations_(node, wsm, level, start_matrix);
    for (const auto &child: node.GetAllChildren())
        PrintLocationsRecursive_(*child, wsm, level + 1, ctm);
}

static Matrix4f PrintMatrices_(const SG::Node &node, int level,
                               const Matrix4f &start_matrix) {
    const Matrix4f mm  = node.GetModelMatrix();
    const Matrix4f ctm = start_matrix * mm;

    std::cout << Indent_(level) << GetDesc_(node) << "\n"
              << Indent_(level, false) << "   L" << mm  << "\n"
              << Indent_(level, false) << "   W" << ctm << "\n";

    return ctm;
}

static void PrintMatricesRecursive_(const SG::Node &node, int level,
                                    const Matrix4f &start_matrix) {
    const Matrix4f ctm = PrintMatrices_(node, level, start_matrix);
    for (const auto &child: node.GetAllChildren())
        PrintMatricesRecursive_(*child, level + 1, ctm);
}

static void PrintTransformFields_(const SG::Node &node, int level) {
    const Str ind = Indent_(level + 1);
    if (node.GetScale() != Vector3f(1, 1, 1))
        std::cout << ind << "scale:       " << node.GetScale() << "\n";
    if (! node.GetRotation().IsIdentity())
        std::cout << ind << "rotation:    " << node.GetRotation() << "\n";
    if (node.GetTranslation() != Vector3f::Zero())
        std::cout << ind << "translation: " << node.GetTranslation() << "\n";
}

static void PrintTransforms_(const SG::Node &node, int level) {
    std::cout << Indent_(level) << GetDesc_(node) << "\n";
    PrintTransformFields_(node, level);
}

static void PrintTransformsRecursive_(const SG::Node &node, int level) {
    PrintTransforms_(node, level);
    for (const auto &child: node.GetAllChildren())
        PrintTransformsRecursive_(*child, level + 1);
}

static bool PrintNodesAndShapes_(const SG::Node &node, int level, bool is_extra,
                                 std::unordered_set<const SG::Object *> &done) {
    const bool was_node_seen = done.find(&node) != done.end();
    const Str extra = is_extra ? " [EXTRA]" : "";
    std::cout << Indent_(level);
    if (was_node_seen) {
        if (node.GetName().empty())
            std::cout << "USE <" << Util::ToString(&node) << ">";
        else
            std::cout << "USE '" << node.GetName() << "'";
        std::cout << extra << "\n";
    }
    else {
        done.insert(&node);
        std::cout << GetDesc_(node) << extra << "\n";

        for (const auto &shape: node.GetShapes()) {
            const bool was_shape_seen = done.find(shape.get()) != done.end();
            if (! was_shape_seen)
                done.insert(shape.get());
            std::cout << Indent_(level + 1)
                      << (was_shape_seen ? "USE " : "")
                      << GetDesc_(*shape) << "\n";
        }
    }
    return ! was_node_seen;
}

static void PrintNodesAndShapesRecursive_(
    const SG::Node &node, int level, bool is_extra,
    std::unordered_set<const SG::Object *> &done) {
    if (PrintNodesAndShapes_(node, level, is_extra, done)) {
        for (const auto &child: node.GetChildren())
            PrintNodesAndShapesRecursive_(*child, level + 1, false, done);
        for (const auto &child: node.GetExtraChildren())
            PrintNodesAndShapesRecursive_(*child, level + 1, true, done);
    }
}

static void PrintPaneTree_(const Pane &pane, int level, bool is_brief) {
    std::cout << Indent_(level) << pane.ToString(is_brief) << "\n";

    for (const auto &subpane: pane.GetSubPanes())
        PrintPaneTree_(*subpane, level + 1, is_brief);

    // Special case for DropdownPane.
    if (const auto *dp = dynamic_cast<const DropdownPane *>(&pane))
        PrintPaneTree_(dp->GetMenuPane(), level + 1, is_brief);
}

static void PrintModelTree_(const SG::Scene &scene, const Model &model,
                            bool is_full) {
    // Compute mesh bounds and bottom center.
    const auto bounds = ComputeMeshBounds(model.GetCurrentMesh());
    const SG::CoordConv cc(SG::FindNodePathInScene(scene, model));
    const auto bottom_center =
        cc.ObjectToRoot(bounds.GetFaceCenter(Bounds::Face::kBottom));

    const int level = model.GetLevel();
    std::cout << Indent_(level) << GetDesc_(model)
              << " " << Util::EnumName(model.GetStatus()) << "\n";
    std::cout << Indent_(level + 1) << "mesh bounds: " << bounds << "\n";
    std::cout << Indent_(level + 1) << "bottom center at: "
              << bottom_center << "\n";

    // Print the complexity.
    const float complex = model.GetComplexity();
    if (complex != TK::kModelComplexity)
        std::cout << Indent_(level + 1) << "complexity:  " << complex << "\n";

    PrintTransformFields_(model, level);
    // Also print the center offset if any.
    const Vector3f &offset = model.GetObjectCenterOffset();
    if (offset != Vector3f::Zero())
        std::cout << Indent_(level + 1) << "offset:      " << offset << "\n";

    // Write out all Model fields (except those that refer to other Models).
    if (is_full) {
        Parser::Writer writer(std::cout);
        for (const auto &f: model.GetModelFields()) {
            if (! dynamic_cast<const Parser::ObjectField<Model> *>(f) &&
                ! dynamic_cast<const Parser::ObjectListField<Model> *>(f)) {
                std::cout << Indent_(level + 1);
                writer.WriteField(*f);
            }
        }
    }

    // Recurse on ParentModels.
    if (const auto *parent = dynamic_cast<const ParentModel *>(&model)) {
        for (size_t i = 0; i < parent->GetChildModelCount(); ++i)
            PrintModelTree_(scene, *parent->GetChildModel(i), is_full);
    }
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

namespace Debug {

void PrintObject(const Parser::Object &obj, bool print_addresses) {
    Parser::Writer writer(std::cout);
    writer.SetAddressFlag(print_addresses);
    writer.WriteObject(obj);
}

void PrintCommands(const CommandList &command_list) {
    Surrounder_ surrounder;
    PrintObject(command_list, false);
    const auto index = command_list.GetCurrentIndex();
    std::cout << "# Index " << index << " = "
              << (index < command_list.GetCommandCount() ?
                  command_list.GetCommand(index)->GetDesc() : "<END>") << "\n";
}

void PrintScene(const SG::Scene &scene, bool print_addresses) {
    Surrounder_ surrounder;
    PrintObject(scene, print_addresses);
}

void PrintGraph(const SG::Node &root) {
    Surrounder_ surrounder;
    Parser::Writer writer(std::cout);
    writer.SetAddressFlag(true);
    writer.WriteObject(root);
}

void PrintGraphOnPath(const SG::NodePath &path, bool print_below) {
    Surrounder_ surrounder;
    Parser::Writer writer(std::cout);
    writer.SetAddressFlag(true);
    if (PrintPath_(path)) {
        PathLimiter_ pl(path, print_below);
        auto write_it = [&](const Parser::Object &obj, bool enter){
            const SG::Node *node = dynamic_cast<const SG::Node *>(&obj);
            bool ret = true;
            if (node) { // Non-Nodes are always written.
                if (enter)
                    ret = pl.Push(*node);
                else
                    pl.Pop();
            }
            return ret;
        };
        writer.WriteObjectConditional(*path.front(), write_it);
    }
}

void PrintBounds(const SG::Node &root, const Matrix4f &wsm) {
    Surrounder_ surrounder;
    PrintBoundsRecursive_(root, wsm, 0, Matrix4f::Identity());
}

void PrintBoundsOnPath(const SG::NodePath &path, const Matrix4f &wsm,
                       bool print_below) {
    Surrounder_ surrounder;
    if (PrintPath_(path)) {
        int level = 0;
        Matrix4f ctm = Matrix4f::Identity();
        for (auto &node: path) {
            if (node == path.back() && print_below)
                PrintBoundsRecursive_(*node, wsm, level, ctm);
            else
                ctm = PrintBounds_(*node, wsm, level++, ctm);
        }
    }
}

void PrintLocations(const SG::Node &root, const Matrix4f &wsm) {
    Surrounder_ surrounder;
    PrintLocationsRecursive_(root, wsm, 0, Matrix4f::Identity());
}

void PrintLocationsOnPath(const SG::NodePath &path, const Matrix4f &wsm,
                       bool print_below) {
    Surrounder_ surrounder;
    if (PrintPath_(path)) {
        int level = 0;
        Matrix4f ctm = Matrix4f::Identity();
        for (auto &node: path) {
            if (node == path.back() && print_below)
                PrintLocationsRecursive_(*node, wsm, level, ctm);
            else
                ctm = PrintLocations_(*node, wsm, level++, ctm);
        }
    }
}

void PrintMatrices(const SG::Node &root) {
    Surrounder_ surrounder;
    PrintMatricesRecursive_(root, 0, Matrix4f::Identity());
}

void PrintMatricesOnPath(const SG::NodePath &path, bool print_below) {
    Surrounder_ surrounder;
    if (PrintPath_(path)) {
        int level = 0;
        Matrix4f ctm = Matrix4f::Identity();
        for (auto &node: path) {
            if (node == path.back() && print_below)
                PrintMatricesRecursive_(*node, level, ctm);
            else
                ctm = PrintMatrices_(*node, level++, ctm);
        }
    }
}

void PrintTransforms(const SG::Node &root) {
    Surrounder_ surrounder;
    PrintTransformsRecursive_(root, 0);
}

void PrintTransformsOnPath(const SG::NodePath &path, bool print_below) {
    Surrounder_ surrounder;
    if (PrintPath_(path)) {
        int level = 0;
        for (auto &node: path) {
            if (node == path.back() && print_below)
                PrintTransformsRecursive_(*node, level);
            else
                PrintTransforms_(*node, level++);
        }
    }
}

void PrintNodesAndShapes(const SG::Node &root) {
    Surrounder_ surrounder;
    std::unordered_set<const SG::Object *> done;
    PrintNodesAndShapesRecursive_(root, 0, false, done);
}

void PrintNodesAndShapesOnPath(const SG::NodePath &path, bool print_below) {
    Surrounder_ surrounder;
    if (PrintPath_(path)) {
        std::unordered_set<const SG::Object *> done;
        int level = 0;
        for (auto &node: path) {
            if (node == path.back() && print_below)
                PrintNodesAndShapesRecursive_(*node, level, false, done);
            else
                PrintNodesAndShapes_(*node, level++, false, done);
        }
    }
}

void PrintBoard(const Board &board, bool is_brief) {
    Surrounder_ surrounder;
    std::cout << board.GetDesc() << " with behavior "
              << Util::EnumName(board.GetBehavior())
              << (board.IsFloating() ? " (floating):\n" : ":\n");
    PrintPaneTree_(*board.GetCurrentPanel()->GetPane(), 0, is_brief);
}

void PrintPaneTree(const Pane &root, bool is_brief) {
    Surrounder_ surrounder;
    PrintPaneTree_(root, 0, is_brief);
}

void PrintModels(const SG::Scene &scene, const Model &root, bool is_full) {
    Surrounder_ surrounder;
    PrintModelTree_(scene, root, is_full);
}

void PrintViewInfo(const SG::Gantry &gantry, const Frustum &frustum,
                   const SG::Node &stage) {
    Surrounder_ surrounder;
    std::cout << "Gantry: height = " << gantry.GetHeight() << "\n";
    std::cout << "Frustum:\n" << frustum << "\n";
    std::cout << "Proj: " << GetProjectionMatrix(frustum) << "\n";
    std::cout << "View: " << GetViewMatrix(frustum) << "\n";
    std::cout << "Stage: scale=" << Util::ToString(stage.GetScale(), .001f)
              << " rot=" << Util::ToString(stage.GetRotation()) << "\n";
}

}  // namespace Debug

#endif
