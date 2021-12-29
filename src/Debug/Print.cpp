#include "Debug/Print.h"

#include <iostream>
#include <unordered_set>

#include "Math/Linear.h"
#include "Math/Types.h"
#include "Panes/ContainerPane.h"
#include "Panes/Pane.h"
#include "Parser/Writer.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "SG/Scene.h"
#include "SG/Shape.h"
#include "Util/Assert.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

namespace {

static SG::NodePath stage_path_;
static SG::NodePath mouse_path_;

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
    for (const auto &child: node.GetChildren())
        PrintNodeBoundsRecursive_(*child, level + 1, ctm);
}

static Matrix4f PrintNodeMatrices_(const SG::Node &node, int level,
                                   const Matrix4f &start_matrix) {
    std::cout << Indent_(level) << node.GetDesc() << " "
              << node.GetModelMatrix() << "\n";

    const Matrix4f ctm = start_matrix * node.GetModelMatrix();
    std::cout << Indent_(level, false) << "        => " << ctm << "\n";

    return ctm;
}

static void PrintNodeMatricesRecursive_(const SG::Node &node, int level,
                                        const Matrix4f &start_matrix) {
    const Matrix4f ctm = PrintNodeMatrices_(node, level, start_matrix);
    for (const auto &child: node.GetChildren())
        PrintNodeMatrices_(*child, level + 1, ctm);
}

static void PrintNodesAndShapes_(const SG::Node &node, int level,
                                 std::unordered_set<const SG::Object *> &done) {
    std::cout << Indent_(level) << node.GetDesc();
    if (! node.IsEnabled(SG::Node::Flag::kTraversal))
        std::cout << " (DISABLED)";
    if (done.find(&node) != done.end()) {
        std::cout << ";\n";
    }
    else {
        done.insert(&node);
        std::cout << "\n";
        for (const auto &shape: node.GetShapes()) {
            std::cout << Indent_(level + 1) << shape->GetDesc();
            if (done.find(shape.get()) != done.end()) {
                std::cout << ";\n";
            }
            else {
                done.insert(shape.get());
                std::cout << "\n";
            }
        }
        for (const auto &child: node.GetChildren())
            PrintNodesAndShapes_(*child, level + 1, done);
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

void SetStagePath(const SG::NodePath &path) {
    stage_path_ = path;
}

void SetMousePath(const SG::NodePath &path) {
    mouse_path_ = path;
}

void PrintScene(const SG::Scene &scene) {
    std::cout << "--------------------------------------------------\n";
    Parser::Writer writer;
    writer.SetAddressFlag(true);
    writer.WriteObject(scene, std::cout);
    std::cout << "--------------------------------------------------\n";
}

void PrintNodeGraph(const SG::Node &root, bool use_path) {
    std::cout << "--------------------------------------------------\n";
    Parser::Writer writer;
    writer.SetAddressFlag(true);
    if (use_path) {
        if (mouse_path_.empty()) {
            std::cout << "<EMPTY PATH>\n";
        }
        else {
            auto is_in_path = [&](const Parser::Object &obj) -> bool{
                const SG::Node *node = dynamic_cast<const SG::Node *>(&obj);
                if (! node)
                    return true;
                for (auto &path_node: mouse_path_)
                    if (path_node.get() == node)
                        return true;
                return false;
            };
            writer.WriteObjectConditional(*mouse_path_.front(), is_in_path,
                                          std::cout);
        }
    }
    else {
        writer.WriteObject(root, std::cout);
    }
    std::cout << "--------------------------------------------------\n";
}

void PrintNodeBounds(const SG::Node &root, bool use_path) {
    std::cout << "--------------------------------------------------\n";
    if (use_path) {
        if (mouse_path_.empty()) {
            std::cout << "<EMPTY PATH>\n";
            return;
        }
        std::cout << "--------------------------------------------------\n";
        int level = 0;
        Matrix4f ctm = Matrix4f::Identity();
        for (auto &node: mouse_path_)
            ctm *= PrintNodeBounds_(*node, level++, ctm);
    }
    else {
        PrintNodeBoundsRecursive_(root, 0, Matrix4f::Identity());
    }
    std::cout << "--------------------------------------------------\n";
}

void PrintNodeMatrices(const SG::Node &root, bool use_path) {
    std::cout << "--------------------------------------------------\n";
    if (use_path) {
        if (mouse_path_.empty()) {
            std::cout << "<EMPTY PATH>\n";
            return;
        }
        int level = 0;
        Matrix4f ctm = Matrix4f::Identity();
        for (auto &node: mouse_path_)
            ctm *= PrintNodeMatrices_(*node, level++, ctm);
    }
    else {
        PrintNodeMatricesRecursive_(root, 0, Matrix4f::Identity());
    }
    std::cout << "--------------------------------------------------\n";
}

void PrintNodesAndShapes(const SG::Node &root) {
    std::cout << "--------------------------------------------------\n";
    std::unordered_set<const SG::Object *> done;
    PrintNodesAndShapes_(root, 0, done);
    std::cout << "--------------------------------------------------\n";
}

void PrintPaneTree(const Pane &root) {
    std::cout << "--------------------------------------------------\n";
    PrintPaneTree_(root, 0);
    std::cout << "--------------------------------------------------\n";
}

}  // namespace Debug
