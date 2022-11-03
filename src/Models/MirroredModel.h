#pragma once

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Models/ConvertedModel.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(MirroredModel);

/// MirroredModel is a derived ConvertedModel class that represents a Model
/// that has been mirrored across one or more planes through its center. Each
/// plane is specified as a normal vector in object coordinates.
///
/// \ingroup Models
class MirroredModel : public ConvertedModel {
  public:
    /// Adds a plane through the center (specified by normal in object
    /// coordinates) to mirror across. The vector is normalized if necessary.
    void AddPlaneNormal(const Vector3f &object_normal);

    /// Removes the last plane normal added. This should not be called if there
    /// is only one.
    void RemoveLastPlaneNormal();

    /// Returns the current mirroring plane normals. This may be an empty
    /// vector.
    const std::vector<Vector3f> & GetPlaneNormals() const {
        return plane_normals_;
    }

  protected:
    MirroredModel() {}
    virtual void AddFields() override;
    virtual TriMesh BuildMesh() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::VField<Vector3f> plane_normals_;
    ///@}

    friend class Parser::Registry;
};
