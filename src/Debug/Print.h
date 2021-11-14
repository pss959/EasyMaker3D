#pragma once

#ifdef DEBUG  // Don't include this in release builds.

namespace SG { class Node; class Scene; }
class Pane;

/// \name Debug Printing Functions
/// Each of these prints some useful debugging information to standard output.
///@{

/// Prints a scene.
void PrintScene(const SG::Scene &scene);

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

#endif
