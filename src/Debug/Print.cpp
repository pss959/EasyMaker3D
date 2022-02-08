#include "Debug/Print.h"

#include <iostream>
#include <stack>
#include <unordered_set>

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/uniform.h>
#include <ion/gfx/uniformblock.h>

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

using IonNode_ = ion::gfx::Node;

typedef std::vector<const IonNode_ *> IonPath_;

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

static CoordConv       coord_conv_;
static SceneContextPtr scene_context_;
static SG::NodePath    limit_path_;

static std::string Indent_(int level, bool add_horiz = true) {
    std::string s;
    for (int i = 1; i < level; ++i)
        s += "| ";
    if (level >= 1)
        s += add_horiz ? "|-" : "| ";
    return s;
}

static std::string GetDesc_(const SG::Object &obj) {
    // Start with regular description string.
    std::string s = obj.GetDesc();

    // If a Node, add disabled flags.
    if (const SG::Node *node = dynamic_cast<const SG::Node *>(&obj)) {
        const auto flags = node->GetDisabledFlags();
        if (flags.HasAny())
            s += " [" + flags.ToString() + "]";
    }
    return s;
}

static Matrix4f PrintNodeBounds_(const SG::Node &node, int level,
                                 const Matrix4f &start_matrix) {
    std::string indent = Indent_(level);
    const Matrix4f ctm = start_matrix * node.GetModelMatrix();
    const Matrix4f wsm = coord_conv_.GetWorldToStageMatrix();

    auto print_bounds = [indent, ctm, wsm](const SG::Object &obj,
                                           const Bounds &bounds,
                                           const std::string &extra){
        const Bounds wbounds = TransformBounds(bounds,  ctm);
        const Bounds sbounds = TransformBounds(wbounds, wsm);
        std::cout << indent << extra << GetDesc_(obj) << "\n"
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

    std::cout << Indent_(level) << GetDesc_(node) << "\n"
              << Indent_(level, false) << "   L" << mm  << "\n"
              << Indent_(level, false) << "   W" << ctm << "\n";

    return ctm;
}

static void PrintNodeMatricesRecursive_(const SG::Node &node, int level,
                                        const Matrix4f &start_matrix) {
    const Matrix4f ctm = PrintNodeMatrices_(node, level, start_matrix);
    for (const auto &child: node.GetAllChildren())
        PrintNodeMatricesRecursive_(*child, level + 1, ctm);
}

static void PrintNodeTransforms_(const SG::Node &node, int level) {
    std::cout << Indent_(level) << GetDesc_(node) << "\n";
    const std::string ind = Indent_(level + 1);
    if (node.GetScale() != Vector3f(1, 1, 1))
        std::cout << ind << "scale:       " << node.GetScale() << "\n";
    if (! node.GetRotation().IsIdentity())
        std::cout << ind << "rotation:    " << node.GetRotation() << "\n";
    if (node.GetTranslation() != Vector3f::Zero())
        std::cout << ind << "translation: " << node.GetTranslation() << "\n";
}

static void PrintNodeTransformsRecursive_(const SG::Node &node, int level) {
    PrintNodeTransforms_(node, level);
    for (const auto &child: node.GetAllChildren())
        PrintNodeTransformsRecursive_(*child, level + 1);
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
        std::cout << GetDesc_(node);
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

static void PrintIonNode_(const IonNode_ &node) {
    std::cout << (node.GetLabel().empty() ? "*" : node.GetLabel());
}

static Matrix4f FindIonMatrix_(const IonNode_ &node) {
    // Look through Ion UniformBlocks for the uModelMatrix uniform.
    for (const auto &block: node.GetUniformBlocks()) {
        for (const auto &uni: block->GetUniforms()) {
            const auto &specs = uni.GetRegistry().GetSpecs<ion::gfx::Uniform>();
            if (specs[uni.GetIndexInRegistry()].name == "uModelMatrix") {
                return uni.GetValue<Matrix4f>();
            }
        }
    }
    // Not found.
    return Matrix4f::Identity();
}

static Matrix4f PrintIonMatrix_(const IonNode_ &node, int level,
                                const Matrix4f &start_matrix) {
    const Matrix4f mm = FindIonMatrix_(node);
    const Matrix4f ctm = start_matrix * mm;

    std::cout << Indent_(level);
    PrintIonNode_(node);
    std::cout << "\n"
              << Indent_(level, false) << "   L" << mm  << "\n"
              << Indent_(level, false) << "   W" << ctm << "\n";

    return ctm;
}

static void PrintIonMatrixRecursive_(const IonNode_ &node, int level,
                                     const Matrix4f &start_matrix) {
    const Matrix4f ctm = PrintIonMatrix_(node, level, start_matrix);
    for (const auto &child: node.GetChildren())
        PrintIonMatrixRecursive_(*child, level + 1, ctm);
}

static void PrintIonMatricesOnPath_(const IonPath_ &path) {
    ASSERT(! path.empty());
    std::cout << "Found Ion path: ";
    for (const auto &node: path) {
        if (node != path.front())
            std::cout << "/";
        PrintIonNode_(*node);
    }
    std::cout << "\n";

    int level = 0;
    Matrix4f ctm = Matrix4f::Identity();
    for (const auto &node: path) {
        if (node == path.back())
            PrintIonMatrixRecursive_(*node, level, ctm);
        else
            ctm *= PrintIonMatrix_(*node, level++, ctm);
    }
}

static void PrintIonPathMatrices_(const IonPath_ &path,
                                  const IonNode_ &target) {
    ASSERT(! path.empty());
    const IonNode_ *node = path.back();

    if (node == &target)
        PrintIonMatricesOnPath_(path);

    // Recurse.
    IonPath_ new_path = path;
    for (const auto &child: node->GetChildren()) {
        new_path.push_back(child.Get());
        PrintIonPathMatrices_(new_path, target);
        new_path.pop_back();
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

void SetSceneContext(const SceneContextPtr &scene_context) {
    ASSERT(scene_context);
    scene_context_ = scene_context;
    coord_conv_.SetStagePath(
        SG::FindNodePathInScene(*scene_context_->scene, "Stage"));
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

void PrintNodeTransforms(const SG::Node &root, bool use_path) {
    std::cout << "--------------------------------------------------\n";
    if (use_path) {
        if (limit_path_.empty()) {
            std::cout << "<EMPTY PATH>\n";
            return;
        }
        std::cout << "PATH = '" << limit_path_.ToString() << "'\n";
        int level = 0;
        for (auto &node: limit_path_) {
            if (node == limit_path_.back())
                PrintNodeTransformsRecursive_(*node, level);
            else
                PrintNodeTransforms_(*node, level++);
        }
    }
    else {
        PrintNodeTransformsRecursive_(root, 0);
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

void PrintIonMatrices(const IonNode_ &root, const IonNode_ &target) {
    std::cout << "--------------------------------------------------\n";
    // Search for the target node, printing each path found.
    IonPath_ path;
    path.push_back(&root);
    PrintIonPathMatrices_(path, target);
    std::cout << "--------------------------------------------------\n";
}

void PrintPaneTree(const Pane &root) {
    std::cout << "--------------------------------------------------\n";
    PrintPaneTree_(root, 0);
    std::cout << "--------------------------------------------------\n";
}

void PrintViewInfo(const Frustum &frustum) {
    std::cout << "--------------------------------------------------\n";
    std::cout << "Frustum:\n" << frustum.ToString() << "\n";
    std::cout << "Proj: " << GetProjectionMatrix(frustum) << "\n";
    std::cout << "View: " << GetViewMatrix(frustum) << "\n";
    std::cout << "--------------------------------------------------\n";
}

bool ProcessPrintShortcut(const std::string &key_string) {
    ASSERT(scene_context_);
    ASSERT(scene_context_->scene);
    const SG::Node &root = *scene_context_->scene->GetRootNode();

    if      (key_string == "<Alt>b") {
        PrintNodeBounds(root, false);
    }
    else if (key_string == "<Alt>B") {
        PrintNodeBounds(root, true);
    }
    else if (key_string == "<Alt>f") {
        const auto board =
            SG::FindTypedNodeUnderNode<Board>(root, "FloatingBoard");
        PrintPaneTree(*board->GetPanel()->GetPane());
    }
    else if (key_string == "<Alt>I") {
        if (! limit_path_.empty())
            PrintIonMatrices(*root.GetIonNode(),
                             *limit_path_.back()->GetIonNode());
    }
    else if (key_string == "<Alt>m") {
        PrintNodeMatrices(root, false);
    }
    else if (key_string == "<Alt>M") {
        PrintNodeMatrices(root, true);
    }
    else if (key_string == "<Alt>n") {
        PrintNodesAndShapes(root, false);
    }
    else if (key_string == "<Alt>N") {
        PrintNodesAndShapes(root, true);
    }
    else if (key_string == "<Alt>p") {
        PrintScene(*scene_context_->scene);
    }
    else if (key_string == "<Alt>P") {
        PrintNodeGraph(root, true);
    }
    else if (key_string == "<Alt>t") {
        PrintNodeTransforms(root, false);
    }
    else if (key_string == "<Alt>T") {
        PrintNodeTransforms(root, true);
    }
    else if (key_string == "<Alt>v") {
        PrintViewInfo(scene_context_->frustum);
    }
    else if (key_string == "<Alt>w") {
        const auto board =
            SG::FindTypedNodeUnderNode<Board>(root, "WallBoard");
        PrintPaneTree(*board->GetPanel()->GetPane());
    }
    else {
        return false;
    }
    return true;
}

}  // namespace Debug
