#pragma once

#ifdef DEBUG  // Don't include this in release builds.

#include <string>

#include "SG/NodePath.h"
#include "SG/Typedefs.h"

class Pane;

namespace Debug {

/// Sets the current Scene.
void SetScene(const SG::ScenePtr &scene);

/// Sets a path used to limit printing.
void SetLimitPath(const SG::NodePath &path);

/// \name Debug Printing Functions
/// Each of these prints some useful debugging information to standard output.
/// The functions that take a use_path argument restrict information to nodes
/// on the path set by SetLimitPath() when true is passed.
///@{

/// Prints a scene.
void PrintScene(const SG::Scene &scene);

/// Prints a graph rooted by the given node.
void PrintNodeGraph(const SG::Node &root, bool use_path);

/// Prints an indented tree of bounds of all nodes starting with the given
/// one.
void PrintNodeBounds(const SG::Node &root, bool use_path);

/// Prints an indented tree of nodes with their local and cumulative matrices.
void PrintNodeMatrices(const SG::Node &root, bool use_path);

/// Prints an indented tree of names and types of nodes and shapes.
void PrintNodesAndShapes(const SG::Node &root, bool use_path);

/// Prints an indented tree of Panes in a Pane tree.
void PrintPaneTree(const Pane &root);

/// Processes a shortcut key string to print something. Returns true if
/// something was done.
bool ProcessPrintShortcut(const std::string &key_string);

///@}

}  // namespace Debug

#endif
