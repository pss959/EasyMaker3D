#pragma once

#ifdef DEBUG  // Don't include this in release builds.

namespace SG { class Node; class NodePath; class Scene; }
class Pane;

namespace Debug {

/// Sets the path from the root of the scene to the Stage. This is used for
/// computing stage coordinates.
void SetStagePath(const SG::NodePath &path);

/// \name Debug Printing Functions
/// Each of these prints some useful debugging information to standard output.
///@{

/// Prints a scene.
void PrintScene(const SG::Scene &scene);

/// Prints a graph rooted by the given node.
void PrintNodeGraph(const SG::Node &root);

/// Prints an indented tree of bounds of all nodes starting with the given
/// one.
void PrintNodeBounds(const SG::Node &root);

/// Prints an indented tree of nodes with their local and cumulative matrices.
void PrintNodeMatrices(const SG::Node &root);

/// Prints an indented tree of names and types of nodes and shapes.
void PrintNodesAndShapes(const SG::Node &root);

/// Prints an indented tree of Panes in a Pane tree.
void PrintPaneTree(const Pane &root);

///@}

}  // namespace Debug

#endif
