//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#if ENABLE_DEBUG_FEATURES

#include "Math/Types.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(CommandList);
DECL_SHARED_PTR(LogHandler);
DECL_SHARED_PTR(SceneContext);
namespace SG { class NodePath; }

namespace Debug {

/// \name Debugging Shortcut Setup Functions
/// These functions are used to set up the debug shortcut functions.
///@{

/// Sets the current CommandList for printing commands.
void SetCommandList(const CommandListPtr &command_list);

/// Sets the LogHandler.
void SetLogHandler(const LogHandlerPtr &log_handler);

/// Sets the current SceneContext.
void SetSceneContext(const SceneContextPtr &scene_context);

/// Sets a path used to limit printing.
void SetLimitPath(const SG::NodePath &path);

/// Frees everything up to avoid destruction in the wrong order.
void ShutDown();

///@}

/// Potentially handles a debugging shortcut, returning true if handled.
bool HandleShortcut(const Str &str);

/// Displays the given text string in the debug text area. If the text string
/// is empty, this turns off the text string.
void DisplayDebugText(const Str &text);

/// Displays the debug sphere at the given location with the given diameter,
/// both in stage coordinates.
void DisplayDebugSphereStage(const Point3f &pos, float diameter);

/// Displays the debug sphere at the given location with the given diameter,
/// both in world coordinates.
void DisplayDebugSphereWorld(const Point3f &pos, float diameter);

}  // namespace Debug

#endif
