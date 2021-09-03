#pragma once

#include <ion/gfxutils/shapeutils.h>

#include "SG/TriMeshShape.h"
#include "Util/FilePath.h"

namespace SG {

//! ImportedShape is a derived TriMeshShape that loads its Ion shape from a
//! file. These files are not tracked by the Tracker to save some memory.
class ImportedShape : public TriMeshShape {
  public:
    virtual void AddFields() override;

    //! Returns the path that the shape was read from.
    Util::FilePath GetFilePath() const { return path_.GetValue(); }

    bool             ShouldAddNormals()   const { return add_normals_;    }
    bool             ShouldAddTexCoords() const { return add_texcoords_;  }
    const Vector2i & GetTexDimensions()   const { return tex_dimensions_; }
    const ShapePtr & GetProxyShape()      const { return proxy_shape_;    }

    virtual bool IntersectRay(const Ray &ray, Hit &hit) const override;
    virtual ion::gfx::ShapePtr CreateIonShape() override;

  private:
    //! \name Parsed Fields
    //!@{
    Parser::TField<std::string> path_{"path"};
    Parser::TField<bool>        add_normals_{"add_normals", false};
    Parser::TField<bool>        add_texcoords_{"add_texcoords", false};
    Parser::TField<Vector2i>    tex_dimensions_{"tex_dimensions", {0, 1}};
    Parser::ObjectField<Shape>  proxy_shape_{"proxy_shape"};
    //!@}
};

}  // namespace SG
