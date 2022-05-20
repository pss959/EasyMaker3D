#pragma once

#ifdef DEBUG  // Don't include this in release builds.

#include <string>

#include <ion/gfx/node.h>

#include "Base/Memory.h"

class Model;
class Pane;
struct Frustum;
DECL_SHARED_PTR(LogHandler);
DECL_SHARED_PTR(CommandList);
DECL_SHARED_PTR(SceneContext);
namespace Parser { class Object; }
namespace SG { class Node; class NodePath; class Scene; }

/// \file
/// This file contains functions that help diagnose and debug problems.
///
/// \ingroup Debug

namespace Debug {

/// \name Debugging Setup Functions
/// These functions are used to set up the debug printing functions.
///@{

/// Sets the LogHandler.
void SetLogHandler(const LogHandlerPtr &log_handler);

/// Sets the current CommandList for printing commands.
void SetCommandList(const CommandListPtr &command_list);

/// Sets the current SceneContext.
void SetSceneContext(const SceneContextPtr &scene_context);

/// Sets a path used to limit printing.
void SetLimitPath(const SG::NodePath &path);

/// Frees everything up to avoid destruction in the wrong order.
void ShutDown();

/// \name Debug Printing Functions
/// Each of these prints some useful debugging information to standard output.
/// The functions that take a use_path argument restrict information to nodes
/// on the path set by SetLimitPath() when true is passed.
///@{

/// Prints any type of Parser::Object.
void PrintObject(const Parser::Object &obj);

/// Prints all commands in the CommandList passed to SetCommandList().
void PrintCommands();

/// Prints a scene.
void PrintScene(const SG::Scene &scene);

/// Prints a graph rooted by the given node.
void PrintNodeGraph(const SG::Node &root, bool use_path);

/// Prints an indented tree of bounds of all nodes starting with the given
/// one.
void PrintNodeBounds(const SG::Node &root, bool use_path);

/// Prints an indented tree of nodes with their local and cumulative matrices.
void PrintNodeMatrices(const SG::Node &root, bool use_path);

/// Prints an indented tree of nodes with their modified transform fields.
void PrintNodeTransforms(const SG::Node &root, bool use_path);

/// Prints an indented tree of names and types of nodes and shapes.
void PrintNodesAndShapes(const SG::Node &root, bool use_path);

/// Searches for all paths from the given root Ion Node to the target Ion Node,
/// then prints all individual and cumulative uModelMatrix uniforms along those
/// paths.
void PrintIonMatrices(const ion::gfx::Node &root, const ion::gfx::Node &target);

/// Prints an indented tree of Panes in a Pane tree.
void PrintPaneTree(const Pane &root);

/// Prints an indented tree of Models starting with the given one.
void PrintModels(const Model &model);

/// Prints current viewing information based on the given frustum.
void PrintViewInfo(const Frustum &frustum);

/// Processes a shortcut key string to print something. Returns true if
/// something was done.
bool ProcessPrintShortcut(const std::string &key_string);

///@}

/// \name Debug Display Functions
/// Each of these display useful debugging information in the debug_text
/// Node. They require SetSceneContext() to have been called.
///@{

/// Displays the given text string.
void DisplayText(const std::string &text);

}  // namespace Debug

#endif
