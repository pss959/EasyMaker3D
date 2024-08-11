//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Types.h"
#include "Models/ConvertedModel.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(MirroredModel);

/// MirroredModel is a derived ConvertedModel class that represents a Model
/// that has been mirrored across a plane through the center of the Model. The
/// plane normal is specified in object coordinates of the operand Model.
///
/// The MirroredModel mirrors the operand Model's mesh around the center, but
/// the mirroring usually results in a change in translation.  Therefore, the
/// translation in the MirroredModel usually differs from the translation in
/// the operand.
///
/// \ingroup Models
class MirroredModel : public ConvertedModel {
  public:
    /// Returns the default mirroring plane normal.
    static Vector3f GetDefaultPlaneNormal() { return Vector3f::AxisX(); }

    /// Sets the plane normal (specified in object coordinates of the operand
    /// Model) to mirror across.
    void SetPlaneNormal(const Vector3f &plane_normal);

    /// Returns the mirroring plane normal (in object coordinates of the
    /// operand Model).
    const Vector3f & GetPlaneNormal() const { return plane_normal_; }

  protected:
    MirroredModel() {}
    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;
    virtual TriMesh ConvertMesh(const TriMesh &mesh) override;

    /// Overrides this to deal with the difference in translation.
    virtual void SyncTransformsFromOperand(const Model &operand) override;

    /// Overrides this to deal with the difference in translation.
    virtual void SyncTransformsToOperand(Model &operand) const override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::TField<Vector3f> plane_normal_;
    ///@}

    friend class Parser::Registry;
};
