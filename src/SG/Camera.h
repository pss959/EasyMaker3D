#pragma once

#include <string>
#include <vector>

#include "Parser/ObjectSpec.h"
#include "SG/Object.h"
#include "SG/Math.h"

class Frustum;

namespace SG {

//! The Camera class represents a perspective camera used to view a scene.  The
//! default position is at (0,0,-10) and the default orientation is looking
//! along -Z with +Y as the up direction. The default vertical field of view
//! angle is 60 degrees. The default near and far values are .01 and 20,
//! respectively.
class Camera : public Object {
  public:
    //! The default constructor sets some reasonable values.
    Camera();

    const Vector3f &  GetPosition()    const { return position_;    }
    const Rotationf & GetOrientation() const { return orientation_; }
    const Anglef &    GetFOV()         const { return fov_;         }
    float             GetNear()        const { return near_;        }
    float             GetFar()         const { return far_;         }

    //! Returns a Frustum representing the Camera view. The viewport aspect
    //! ratio is supplied to determine the horizontal field of view angles.
    Frustum BuildFrustum(float aspect) const;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    //! \name Parsed Fields
    //!@{
    Vector3f  position_;     //!< Position of the camera in 3D coordinates.
    Rotationf orientation_;  //!< Rotation from canonical orientation.
    Anglef    fov_;          //!< Vertical field of view angle.
    float     near_;         //!< Distance to near plane.
    float     far_;          //!< Distance to far plane.
    //!@}
};

}  // namespace SG
