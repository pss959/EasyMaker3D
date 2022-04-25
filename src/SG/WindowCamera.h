#pragma once

#include "Math/Types.h"
#include "Memory.h"
#include "SG/Camera.h"

class Frustum;

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(WindowCamera);

/// WindowCamera is a derived Camera class that represents a perspective camera
/// used to view a scene in a 2D window (as opposed to in VR). It has fields
/// defining the original frustum.
///
/// The default position is at (0,0,-10) and the default orientation is looking
/// along -Z with +Y as the up direction. The default vertical field of view
/// angle is 60 degrees. The default near and far values are .01 and 20,
/// respectively.
///
/// \ingroup SG
class WindowCamera : public Camera {
  public:
    const Point3f &   GetPosition()    const { return position_;    }
    const Rotationf & GetOrientation() const { return orientation_; }
    const Anglef &    GetFOV()         const { return fov_;         }
    float             GetNear()        const { return near_;        }
    float             GetFar()         const { return far_;         }

    /// Sets the position.
    void SetPosition(const Point3f &pos) { position_ = pos; }

    /// Sets the orientation.
    void SetOrientation(const Rotationf &rot) { orientation_ = rot; }

    /// Sets the field of view angle.
    void SetFOV(const Anglef &fov) { fov_ = fov; }

    /// Sets the near and far distances.
    void SetNearAndFar(float nr, float fr) { near_ = nr; far_ = fr; }

    /// Convenience that returns the view direction.
    Vector3f GetViewDirection() const {
        return orientation_.GetValue() * -Vector3f::AxisZ();
    }

  protected:
    WindowCamera() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Point3f>   position_{"position", {0, 0, -10}};
    Parser::TField<Rotationf> orientation_{"orientation"};
    Parser::TField<Anglef>    fov_{"fov", Anglef::FromDegrees(60)};
    Parser::TField<float>     near_{"near", .01f};
    Parser::TField<float>     far_{"far", 20.f};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
