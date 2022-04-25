#pragma once

#include "Math/Types.h"
#include "Memory.h"
#include "SG/Object.h"

namespace SG {

DECL_SHARED_PTR(Camera);

/// Camera is an abstract base class representing a perspective camera used to
/// view a scene. The only interface it has is to store the height passed to it
/// from a Gantry.
///
/// \ingroup SG
class Camera : public Object {
  public:
    /// Allows the gantry to set the relative height of the camera. The base
    /// class defines this to just store the height.
    virtual void SetHeight(float height) { height_ = height; }

    /// Returns the saved height.
    float GetHeight() const { return height_; }

  private:
    /// Saves the height passed to it from the gantry. This should be added to
    /// whatever else is used to position the camera.
    float height_ = 0;
};

}  // namespace SG
