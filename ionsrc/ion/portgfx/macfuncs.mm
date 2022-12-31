// This file exists because Macos requires certain (deprecated) calls involving
// OpenGL to be made from Objective-C or Objective-C++.

#include <AppKit/NSOpenGL.h>

uintptr_t MacGetCurrentGlContextId() {
  return reinterpret_cast<uintptr_t>([NSOpenGLContext currentContext]);
}
