#pragma once

#if ENABLE_DEBUG_FEATURES

#include "Math/Types.h"
#include "Util/Memory.h"

class Board;
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
void PrintObject(const Parser::Object &obj, bool print_addresses = false);

/// Prints all commands in a CommandList.
void PrintCommands(const CommandList &command_list);

/// Prints a scene.
void PrintScene(const SG::Scene &scene, bool print_addresses = false);

/// Prints a graph rooted by the given node.
void PrintGraph(const SG::Node &root);

/// Prints nodes on the given path.
void PrintGraphOnPath(const SG::NodePath &path, bool print_below = true);

/// Prints an indented tree of bounds of all nodes starting with the given one.
/// A matrix used to convert from world to stage coordinates is supplied.
void PrintBounds(const SG::Node &root, const Matrix4f &wsm);

/// Prints an indented tree of bounds of all nodes on the given path.  A matrix
/// used to convert from world to stage coordinates is supplied.
void PrintBoundsOnPath(const SG::NodePath &path, const Matrix4f &wsm,
                       bool print_below = true);

/// Prints an indented tree of locations of all nodes starting with the given
/// one. A matrix used to convert from world to stage coordinates is supplied.
void PrintLocations(const SG::Node &root, const Matrix4f &wsm);

/// Prints an indented tree of locations of all nodes on the given path. A
/// matrix used to convert from world to stage coordinates is supplied.
void PrintLocationsOnPath(const SG::NodePath &path, const Matrix4f &wsm,
                          bool print_below = true);

/// Prints an indented tree of nodes with their local and cumulative matrices.
void PrintMatrices(const SG::Node &root);

/// Prints an indented tree of nodes with their local and cumulative matrices.
void PrintMatricesOnPath(const SG::NodePath &path, bool print_below = true);

/// Prints an indented tree of nodes with all modified transform fields.
void PrintTransforms(const SG::Node &root);

/// Prints an indented tree of nodes on the given path with all modified
/// transform fields.
void PrintTransformsOnPath(const SG::NodePath &path, bool print_below = true);

/// Prints an indented tree of names and types of nodes and shapes.
void PrintNodesAndShapes(const SG::Node &root);

/// Prints an indented tree of names and types of nodes and shapes on the given
/// path.
void PrintNodesAndShapesOnPath(const SG::NodePath &path,
                               bool print_below = true);

/// Prints information about the given Board along with indented tree of Panes
/// displayed in it. If \p is_brief is true, this just prints the Panes with
/// their name and current layout position.
void PrintBoard(const Board &board, bool is_brief = true);

/// Prints an indented tree of Panes starting with the given root Pane. If \p
/// is_brief is true, this just prints the Panes with their name and current
/// layout position.
void PrintPaneTree(const Pane &root, bool is_brief = true);

/// Prints an indented tree of Models and their transforms starting with the
/// given root. If \p is_full is true, all other Model-specific fields are
/// printed as well.
void PrintModels(const Model &root, bool is_full);

/// Prints viewing information for the given Frustum and Stage.
void PrintViewInfo(const Frustum &frustum, const SG::Node &stage);

///@}

}  // namespace Debug

#endif
