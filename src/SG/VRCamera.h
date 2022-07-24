#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "SG/Camera.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(VRCamera);

/// VRCamera is a derived Camera class that represents a perspective camera
/// used to view a scene in VR. It defines a base position for positioning the
/// VR camera in the scene.
///
/// \ingroup SG
class VRCamera : public Camera {
  public:
    const Point3f & GetBasePosition() const { return base_position_;    }

    /// Returns the current position taking the height into account.
    virtual Point3f GetCurrentPosition() const override;

  protected:
    VRCamera() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Point3f> base_position_;
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
