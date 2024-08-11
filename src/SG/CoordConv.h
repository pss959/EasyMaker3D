//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Math/Types.h"
#include "SG/NodePath.h"

namespace SG {

/// The CoordConv class helps convert between various coordinate systems using
/// an SG::NodePath that defines those systems.
///
/// <em>Object coordinates</em> represent the coordinate system for a Node
/// before any of its local transform fields or any transform fields for
/// ancestors are applied. This is the coordinate system at the tail of the
/// path.
///
/// <em>Local coordinates</em> represent the coordinate system for a Node after
/// applying its local transform fields, but not any transform fields for
/// ancestors are applied. This is the coordinate system just above the tail of
/// the path.
///
/// <em>Root coordinates</em> are the coordinate system at the root node of
/// the path. Note that depending on the path these coordinates may be
/// equivalent to world coordinates, stage coordinates, or something else.
///
/// \ingroup SG
class CoordConv {
  public:
    /// The constructor is given the SG::NodePath that defines the coordinate
    /// systems. It must not be an empty path.
    explicit CoordConv(const SG::NodePath &path);

    /// \name Matrix Accessors.
    /// Each of these computes and returns a matrix that converts between two
    /// coordinate systems.
    ///@{

    /// Returns a matrix that converts from object to root coordinates.
    Matrix4f GetObjectToRootMatrix() const;

    /// Returns a matrix that converts from root to object coordinates.
    Matrix4f GetRootToObjectMatrix() const;

    /// Returns a matrix that converts from local to root coordinates.
    Matrix4f GetLocalToRootMatrix() const;

    /// Returns a matrix that converts from root to local coordinates.
    Matrix4f GetRootToLocalMatrix() const;

    ///@}

    /// \name Vector and Point Transformation Functions.

    /// Each of these transforms a 3D vector or point between coordinate
    /// systems.
    ///@{

    Vector3f ObjectToRoot(const Vector3f &v) const;
    Vector3f RootToObject(const Vector3f &v) const;
    Vector3f LocalToRoot(const Vector3f &v) const;
    Vector3f RootToLocal(const Vector3f &v) const;

    Point3f ObjectToRoot(const Point3f &p) const;
    Point3f RootToObject(const Point3f &p) const;
    Point3f LocalToRoot(const Point3f &p) const;
    Point3f RootToLocal(const Point3f &p) const;

    ///@}

  private:
    const SG::NodePath path_;
};

}  // namespace SG
