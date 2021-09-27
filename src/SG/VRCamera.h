#pragma once

#include "Math/Types.h"
#include "SG/Camera.h"

namespace Parser { class Registry; }

namespace SG {

/// VRCamera is a derived Camera class that represents a perspective camera
/// used to view a scene in VR. It defines a base position for positioning the
/// VR camera in the scene.
class VRCamera : public Camera {
  public:
    virtual void AddFields() override;

    const Point3f & GetBasePosition() const { return base_position_;    }

    /// Returns the current position taking the height into account.
    Point3f GetPosition() const;

  protected:
    VRCamera() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Point3f> base_position_{"base_position", {0, 0, 0}};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
