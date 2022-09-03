#pragma once

#if ENABLE_DEBUG_FEATURES

#include "Base/Memory.h"
#include "Math/Types.h"

class CommandList;
class Model;
class Pane;
struct Frustum;
namespace Parser { class Object; }
namespace SG { class Node; class NodePath; class Scene; }

/// \file
/// This file contains functions that help diagnose and debug problems.
///
/// \ingroup Debug

namespace Debug {

/// \name Debug Printing Functions
/// Each of these prints some useful debugging information to standard output.
/// All functions that print information about an SG::NodePath handle each node
/// in the path and all nodes under the tail node of the path.
///@{

/// Prints any type of Parser::Object.
void PrintObject(const Parser::Object &obj);

/// Prints all commands in a CommandList.
void PrintCommands(const CommandList &command_list);

/// Prints a scene.
void PrintScene(const SG::Scene &scene);

/// Prints a graph rooted by the given node.
void PrintGraph(const SG::Node &root);

/// Prints nodes on the given path.
void PrintGraphOnPath(const SG::NodePath &path);

/// Prints an indented tree of bounds of all nodes starting with the given one.
/// A matrix used to convert from world to stage coordinates is supplied.
void PrintBounds(const SG::Node &root, const Matrix4f &wsm);

/// Prints an indented tree of bounds of all nodes on the given path.  A matrix
/// used to convert from world to stage coordinates is supplied.
void PrintBoundsOnPath(const SG::NodePath &path, const Matrix4f &wsm);

/// Prints an indented tree of nodes with their local and cumulative matrices.
void PrintMatrices(const SG::Node &root);

/// Prints an indented tree of nodes with their local and cumulative matrices.
void PrintMatricesOnPath(const SG::NodePath &path);

/// Prints an indented tree of nodes with all modified transform fields.
void PrintTransforms(const SG::Node &root);

/// Prints an indented tree of nodes on the given path with all modified
/// transform fields.
void PrintTransformsOnPath(const SG::NodePath &path);

/// Prints an indented tree of names and types of nodes and shapes.
void PrintNodesAndShapes(const SG::Node &root);

/// Prints an indented tree of names and types of nodes and shapes on the given
/// path.
void PrintNodesAndShapesOnPath(const SG::NodePath &path);

/// Prints an indented tree of Panes starting with the given root Pane. If
/// is_brief is true, this just prints the Panes with their name and current
/// layout position.
void PrintPaneTree(const Pane &root, bool is_brief);

/// Prints an indented tree of Models starting with the given root.
void PrintModels(const Model &root);

/// Prints viewing information for the given Frustum.
void PrintViewInfo(const Frustum &frustum);

///@}

}  // namespace Debug

#endif
