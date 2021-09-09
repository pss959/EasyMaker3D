#pragma once

#include <string>
#include <vector>

#include "Math/Types.h"
#include "SG/Object.h"

class Frustum;

namespace SG {

//! The Camera class represents a perspective camera used to view a scene.  The
//! default position is at (0,0,-10) and the default orientation is looking
//! along -Z with +Y as the up direction. The default vertical field of view
//! angle is 60 degrees. The default near and far values are .01 and 20,
//! respectively.
class Camera : public Object {
  public:
    virtual void AddFields() override;

    const Point3f &   GetPosition()    const { return position_;    }
    const Rotationf & GetOrientation() const { return orientation_; }
    const Anglef &    GetFOV()         const { return fov_;         }
    float             GetNear()        const { return near_;        }
    float             GetFar()         const { return far_;         }

    //! Returns a Frustum representing the Camera view. The viewport aspect
    //! ratio is supplied to determine the horizontal field of view angles.
    Frustum BuildFrustum(float aspect) const;

    //! XXXX
    void SetHeight(float height);

  private:
    //! \name Parsed Fields
    //!@{
    Parser::TField<Point3f>   position_{"position", {0, 0, -10}};
    Parser::TField<Rotationf> orientation_{"orientation"};
    Parser::TField<Anglef>    fov_{"fov", Anglef::FromDegrees(60)};
    Parser::TField<float>     near_{"near", .01f};
    Parser::TField<float>     far_{"far", 20.f};
    //!@}

    Point3f starting_position_;

    //! Redefines this to save the parsed starting position.
    virtual void SetFieldParsed(const Parser::Field &field) override;
};

}  // namespace SG
