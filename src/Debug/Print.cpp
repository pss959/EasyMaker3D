#include "Debug/Print.h"

#include <iostream>
#include <stack>
#include <unordered_set>

#include "Items/Board.h"
#include "Math/Linear.h"
#include "Math/Types.h"
#include "Panes/ContainerPane.h"
#include "Panes/Pane.h"
#include "Parser/Writer.h"
#include "SG/Node.h"
#include "SG/Scene.h"
#include "SG/Search.h"
#include "SG/Shape.h"
#include "Util/Assert.h"

namespace {

// ----------------------------------------------------------------------------
// Helper classes.
// ----------------------------------------------------------------------------

// The PathLimiter_ class is used to track a path to limit printing to. It
// returns true if the given Node should be printed, meaning it is either on
// the path or below the last Node in it.
class PathLimiter_ {
  public:
    explicit PathLimiter_(const SG::NodePath &path) : path_(path) {}
    /// Returns true if the given Node should be printed.
    bool Push(const SG::Node &node);
    void Pop();

  private:
    const SG::NodePath          path_;
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
    if (IsInPath_(node)) {
        nodes_.push(&node);
        if (&node == path_.back().get())
            is_under_path_ = true;
        return true;
    }
    return is_under_path_;
}

void PathLimiter_::Pop() {
    if (is_under_path_ && nodes_.top() == path_.back().get())
        is_under_path_ = false;
    nodes_.pop();
}

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

static SG::ScenePtr scene_;
static SG::NodePath stage_path_;
static SG::NodePath limit_path_;

static std::string Indent_(int level, bool add_horiz = true) {
    std::string s;
    for (int i = 1; i < level; ++i)
        s += "| ";
    if (level >= 1)
        s += add_horiz ? "|-" : "| ";
    return s;
}

static Matrix4f PrintNodeBounds_(const SG::Node &node, int level,
                                 const Matrix4f &start_matrix) {
    ASSERT(! stage_path_.empty());
    std::string indent = Indent_(level);
    const Matrix4f ctm = start_matrix * node.GetModelMatrix();
    const Matrix4f wsm = stage_path_.GetToLocalMatrix();

    auto print_bounds = [indent, ctm, wsm](const SG::Object &obj,
                                           const Bounds &bounds,
                                           const std::string &extra){
        const Bounds wbounds = TransformBounds(bounds,  ctm);
        const Bounds sbounds = TransformBounds(wbounds, wsm);
        std::cout << indent << extra << obj.GetDesc() << "\n"
                  << indent << "    LOC: " <<  bounds.ToString() << "\n"
                  << indent << "    STG: " << sbounds.ToString() << "\n"
                  << indent << "    WLD: " << wbounds.ToString() << "\n";
    };

    print_bounds(node, node.GetBounds(), "");
    indent = Indent_(level + 1);

    for (const auto &shape: node.GetShapes())
        print_bounds(*shape, shape->GetBounds(), "--");

    return ctm;
}

static void PrintNodeBoundsRecursive_(const SG::Node &node, int level,
                                      const Matrix4f &start_matrix) {
    const Matrix4f ctm = PrintNodeBounds_(node, level, start_matrix);
    for (const auto &child: node.GetAllChildren())
        PrintNodeBoundsRecursive_(*child, level + 1, ctm);
}

static Matrix4f PrintNodeMatrices_(const SG::Node &node, int level,
                                   const Matrix4f &start_matrix) {
    const Matrix4f mm  = node.GetModelMatrix();
    const Matrix4f ctm = start_matrix * mm;

    std::cout << Indent_(level) << node.GetDesc() << "\n"
              << Indent_(level, false) << "   L" << mm  << "\n"
              << Indent_(level, false) << "   W" << ctm << "\n";

    return ctm;
}

static void PrintNodeMatricesRecursive_(const SG::Node &node, int level,
                                        const Matrix4f &start_matrix) {
    const Matrix4f ctm = PrintNodeMatrices_(node, level, start_matrix);
    for (const auto &child: node.GetAllChildren())
        PrintNodeMatrices_(*child, level + 1, ctm);
}

static bool PrintNodesAndShapes_(const SG::Node &node, int level, bool is_extra,
                                 std::unordered_set<const SG::Object *> &done) {
    const bool was_node_seen = done.find(&node) != done.end();
    const std::string extra = is_extra ? " [EXTRA]" : "";
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
        std::cout << node.GetDesc();
        const auto flags = node.GetDisabledFlags();
        if (flags.HasAny())
            std::cout << " (" << flags.ToString() + ")";
        std::cout << extra << "\n";

        for (const auto &shape: node.GetShapes()) {
            const bool was_shape_seen = done.find(shape.get()) != done.end();
            if (! was_shape_seen)
                done.insert(shape.get());
            std::cout << Indent_(level + 1)
                      << (was_shape_seen ? "USE " : "")
                      << shape->GetDesc() << "\n";
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

static void PrintPaneTree_(const Pane &pane, int level) {
    std::cout << Indent_(level) << pane.ToString() << "\n";

    if (const ContainerPane *cp = dynamic_cast<const ContainerPane *>(&pane))
        for (const auto &subpane: cp->GetPanes())
            PrintPaneTree_(*subpane, level + 1);
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

namespace Debug {

void SetScene(const SG::ScenePtr &scene) {
    ASSERT(scene);
    scene_ = scene;
    stage_path_ = SG::FindNodePathInScene(*scene_, "Stage");
}

void SetLimitPath(const SG::NodePath &path) {
    limit_path_ = path;
}

void PrintScene(const SG::Scene &scene) {
    std::cout << "--------------------------------------------------\n";
    Parser::Writer writer(std::cout);
    writer.SetAddressFlag(true);
    writer.WriteObject(scene);
    std::cout << "--------------------------------------------------\n";
}

void PrintNodeGraph(const SG::Node &root, bool use_path) {
    std::cout << "--------------------------------------------------\n";
    Parser::Writer writer(std::cout);
    writer.SetAddressFlag(true);
    if (use_path) {
        if (limit_path_.empty()) {
            std::cout << "<EMPTY PATH>\n";
            return;
        }
        std::cout << "PATH = '" << limit_path_.ToString() << "'\n";
        PathLimiter_ pl(limit_path_);
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
        writer.WriteObjectConditional(*limit_path_.front(), write_it);
    }
    else {
        writer.WriteObject(root);
    }
    std::cout << "--------------------------------------------------\n";
}

void PrintNodeBounds(const SG::Node &root, bool use_path) {
    std::cout << "--------------------------------------------------\n";
    if (use_path) {
        if (limit_path_.empty()) {
            std::cout << "<EMPTY PATH>\n";
            return;
        }
        std::cout << "PATH = '" << limit_path_.ToString() << "'\n";
        int level = 0;
        Matrix4f ctm = Matrix4f::Identity();
        for (auto &node: limit_path_) {
            if (node == limit_path_.back())
                PrintNodeBoundsRecursive_(*node, level, ctm);
            else
                ctm *= PrintNodeBounds_(*node, level++, ctm);
        }
    }
    else {
        PrintNodeBoundsRecursive_(root, 0, Matrix4f::Identity());
    }
    std::cout << "--------------------------------------------------\n";
}

void PrintNodeMatrices(const SG::Node &root, bool use_path) {
    std::cout << "--------------------------------------------------\n";
    if (use_path) {
        if (limit_path_.empty()) {
            std::cout << "<EMPTY PATH>\n";
            return;
        }
        std::cout << "PATH = '" << limit_path_.ToString() << "'\n";
        int level = 0;
        Matrix4f ctm = Matrix4f::Identity();
        for (auto &node: limit_path_) {
            if (node == limit_path_.back())
                PrintNodeMatricesRecursive_(*node, level, ctm);
            else
                ctm *= PrintNodeMatrices_(*node, level++, ctm);
        }
    }
    else {
        PrintNodeMatricesRecursive_(root, 0, Matrix4f::Identity());
    }
    std::cout << "--------------------------------------------------\n";
}

void PrintNodesAndShapes(const SG::Node &root, bool use_path) {
    std::cout << "--------------------------------------------------\n";
    std::unordered_set<const SG::Object *> done;
    if (use_path) {
        if (limit_path_.empty()) {
            std::cout << "<EMPTY PATH>\n";
            return;
        }
        std::cout << "PATH = '" << limit_path_.ToString() << "'\n";
        int level = 0;
        for (auto &node: limit_path_) {
            if (node == limit_path_.back())
                PrintNodesAndShapesRecursive_(*node, level, false, done);
            else
                PrintNodesAndShapes_(*node, level++, false, done);
        }
    }
    else {
        PrintNodesAndShapesRecursive_(root, 0, false, done);
    }
    std::cout << "--------------------------------------------------\n";
}

void PrintPaneTree(const Pane &root) {
    std::cout << "--------------------------------------------------\n";
    PrintPaneTree_(root, 0);
    std::cout << "--------------------------------------------------\n";
}

bool ProcessPrintShortcut(const std::string &key_string) {
    ASSERT(scene_);
    const SG::Node &root = *scene_->GetRootNode();

    if      (key_string == "<Ctrl>b") {
        PrintNodeBounds(root, false);
    }
    else if (key_string == "<Ctrl>B") {
        PrintNodeBounds(root, true);
    }
    else if (key_string == "<Ctrl>f") {
        const auto board =
            SG::FindTypedNodeUnderNode<Board>(root, "FloatingBoard");
        PrintPaneTree(*board->GetPanel()->GetPane());
    }
    else if (key_string == "<Ctrl>m") {
        PrintNodeMatrices(root, false);
    }
    else if (key_string == "<Ctrl>M") {
        PrintNodeMatrices(root, true);
    }
    else if (key_string == "<Ctrl>n") {
        PrintNodesAndShapes(root, false);
    }
    else if (key_string == "<Ctrl>N") {
        PrintNodesAndShapes(root, true);
    }
    else if (key_string == "<Ctrl>p") {
        PrintScene(*scene_);
    }
    else if (key_string == "<Ctrl>P") {
        PrintNodeGraph(root, true);
    }
    else {
        return false;
    }
    return true;
}

}  // namespace Debug
