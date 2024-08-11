//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <ion/gfxutils/shapeutils.h>

#include "SG/TriMeshShape.h"
#include "Util/FilePath.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(ImportedShape);

/// ImportedShape is a derived TriMeshShape that loads its Ion shape from a
/// file. These files are not tracked by the FileMap to save some memory.
///
/// The ImportedShape may have a proxy Shape that is used for intersection
/// testing instead of testing all triangles in the generated shape. This proxy
/// is not set up to contain Ion info, so it must handle intersection testing
/// without it.
///
/// Another option is to set the \c use_bounds_proxy field to true, in which
/// case the bounds of the shape are used for intersection testing instead of
/// the shape or proxy shape. This can be useful for shapes that are very close
/// to boxes.
///
/// \ingroup SG
class ImportedShape : public TriMeshShape {
  public:
    /// Indicates how to generate surface normals for the shape.
    enum class NormalType {
        kNoNormals,      ///< Do not generate normals.
        kVertexNormals,  ///< Generate normals smoothed at vertices.
        kFaceNormals,    ///< Generate normals based on faces.
    };

    /// Constructs and returns an ImportedShape that uses the given file path
    /// and normal type.
    static ImportedShapePtr CreateFrom(const Str &path, NormalType normal_type);

    /// Returns the path that the shape was read from.
    FilePath GetFilePath() const { return path_.GetValue(); }

    NormalType       GetNormalType()        const { return normal_type_; }
    TexCoordsType    GetTexCoordsType()     const { return tex_coords_type_; }
    const ShapePtr & GetProxyShape()        const { return proxy_shape_;      }
    bool             ShouldUseBoundsProxy() const { return use_bounds_proxy_; }

    virtual bool IntersectRay(const Ray &ray, Hit &hit) const override;

  protected:
    ImportedShape() {}

    virtual void AddFields() override;

    /// Implements this to compute the bounds from the proxy shape, if there is
    /// one.
    virtual Bounds ComputeBounds() const override;
    virtual ion::gfx::ShapePtr CreateSpecificIonShape() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<Str>              path_;
    Parser::EnumField<NormalType>    normal_type_;
    Parser::EnumField<TexCoordsType> tex_coords_type_;
    Parser::ObjectField<Shape>       proxy_shape_;
    Parser::TField<bool>             use_bounds_proxy_;
    ///@}

    /// This is used when use_bounds_proxy_ is true.
    bool IntersectBounds_(const Ray &ray, Hit &hit) const;

    friend class Parser::Registry;
};

}  // namespace SG
