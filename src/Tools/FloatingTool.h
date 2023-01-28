#pragma once

#include "Tools/Tool.h"

/// FloatingTool is a derived Tool class that serves as an abstract base class
/// for tools that float above the current selection. It implements the
/// Update() function to make sure the tool faces the camera at all times.
///
/// \ingroup Tools
class FloatingTool : public Tool {
  public:
    /// Redefines this to rotate the FloatingTool to face the camera.
    virtual void Update() override;
};
