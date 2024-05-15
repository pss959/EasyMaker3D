#pragma once

#include <memory>

/// \file
/// This file can be included to define macros for defining std::shared_ptr
/// types more easily. Using these macros instead of including the headers that
/// define them decouples code better.
///
/// \ingroup Utility

/// Declares the given class and an alias for an std::shared_ptr to the class.
///
/// \ingroup Utility
#define DECL_SHARED_PTR(CL) class CL; using CL ## Ptr = std::shared_ptr<CL>
