#pragma once

#include <ion/gfxutils/shapeutils.h>

#include "Parser/ObjectSpec.h"
#include "SG/TriMeshShape.h"
#include "Util/FilePath.h"

namespace SG {

//! ImportedShape is a derived TriMeshShape that loads its Ion shape from a
//! file. These files are not tracked by the Tracker to save some memory.
class ImportedShape : public TriMeshShape {
  public:
    //! Returns the path that the shape was read from.
    Util::FilePath GetFilePath() const { return path_; }

    bool             ShouldAddNormals()   const { return add_normals_;    }
    bool             ShouldAddTexCoords() const { return add_texcoords_;  }
    const Vector2i & GetTexDimensions()   const { return tex_dimensions_; }
    const ShapePtr & GetProxyShape()      const { return proxy_shape_;    }

    virtual bool IntersectRay(const Ray &ray, Hit &hit) const override;
    virtual ion::gfx::ShapePtr CreateIonShape() override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    //! \name Parsed Fields
    //!@{
    std::string path_;
    bool        add_normals_ = false;
    bool        add_texcoords_ = false;
    Vector2i    tex_dimensions_{0, 1};
    ShapePtr    proxy_shape_;
    //!@}
};

}  // namespace SG
