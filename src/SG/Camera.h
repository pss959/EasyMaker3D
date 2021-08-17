#pragma once

#include <string>
#include <vector>

#include "NParser/FieldSpec.h"
#include "SG/Object.h"
#include "SG/Math.h"

namespace SG {

//! The Camera class represents a perspective camera used to view a scene.  The
//! default orientation for a Camera is looking along -Z with +Y as the up
//! direction.
class Camera : public Object {
  public:
    //! Struct definining field of view angles. The left and down angles are
    //! negative for a symmetric view. Note that for VR, the field is not
    //! necessarily symmetric.
    struct FOV {
        //! Angle from view direction in all 4 directions.
        Anglef left, right, up, down;

        //! The default constructor sets reasonable values.
        FOV();

        //! Constructor that sets up a symmetric FOV using the given full
        //! vertical angle and aspect ratio.
        FOV(const Anglef &vfov, float aspect);
    };

    //! The default constructor sets some reasonable values.
    Camera();

    //! Converts to a string for printing.
    std::string ToString() const;

    static std::vector<NParser::FieldSpec> GetFieldSpecs();

  private:
    // Parsed fields.
    Vector3f  position_;     //!< Position of the camera in 3D coordinates.
    Rotationf orientation_;  //!< Rotation from canonical orientation.
    FOV       fov_;          //!< Field of view angles.
    float     near_;         //!< Distance to near plane.
    float     far_;          //!< Distance to far plane.
    //! This is used for reading a Camera instance - it stores a single angle
    //! that is passed to the FOV constructor.
    Anglef    fov_in_;

    //! Redefines this to set up the fov_ field from fov_in_.
    virtual void Finalize() override;
};

}  // namespace SG
