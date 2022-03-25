#pragma once

#include <memory>

/// \file
/// This file can be included to define macros for defining std::shared_ptr
/// types more easily. Using these macros instead of including the headers that
/// define them decouples code better.

/// Declares the given class and a typedef for an std::shared_ptr to the class.
#define DECL_SHARED_PTR(CL) class CL; typedef std::shared_ptr<CL> CL ## Ptr
