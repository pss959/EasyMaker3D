#pragma once

#include <string>
#include <vector>

/// \file
/// This file contains global type aliases for some basic types to make code
/// briefer.
///
/// Note that this file is force-included from SCons so that it does not need
/// to be explicitly included everywhere.
///
/// \ingroup Utility

using Str    = std::string;
using StrVec = std::vector<std::string>;
