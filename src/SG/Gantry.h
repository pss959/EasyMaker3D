#pragma once

#include <string>
#include <vector>

#include "SG/Camera.h"
#include "SG/Object.h"
#include "SG/Typedefs.h"

class Frustum;

namespace SG {

/// The Gantry class is a special-purpose scene object that contains any number
/// of Camera objects and can move them up and down.
class Gantry : public Object {
  public:
    virtual void AddFields() override;

    /// Returns the cameras managed by the gantry.
    const std::vector<CameraPtr> & GetCameras() const { return cameras_; }

    /// Sets the gantry height. It starts at 0.
    void SetHeight(float height);

    /// Returns the current height.
    float GetHeight() const { return height_; }

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectListField<Camera> cameras_{"cameras"};
    ///@}

    float height_ = 0;   ///< Current height.
};

}  // namespace SG
