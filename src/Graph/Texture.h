#pragma once

#include <string>
#include <vector>

#include "Graph/Object.h"

namespace Graph {

//! The Texture class wraps an Ion Texture.
//!
//! \ingroup Graph
class Texture : public Object {
  public:
    //! Returns the associated Ion texture.
    const ion::gfx::TexturePtr &GetIonTexture() { return i_texture_; }

  private:
    //! Stores all transformation fields.
    Transform transform_;

    ion::gfx::TexturePtr  i_texture_;  //! Associated Ion Texture.

    std::vector<ShapePtr> shapes_;
    std::vector<TexturePtr>  children_;

    //! Overrides this to also set the label in the Ion texture.
    virtual void SetName_(const std::string &name) override;

    //! Enables or disables the texture.
    void SetEnabled_(bool enabled);

    void SetScale_(const ion::math::Vector3f &scale);
    void SetRotation_(const ion::math::Rotationf &rotation);
    void SetTranslation_(const ion::math::Vector3f &translation);

    void SetStateTable_(const ion::gfx::StateTablePtr &state_table);
    void SetShaderProgram_(const ion::gfx::ShaderProgramPtr &program);

    void AddUniform_(const ion::gfx::Uniform &uniform);

    //! Clears the list of child textures.
    void ClearChildren_();

    //! Adds a child.
    void AddChild_(const TexturePtr &child);

    //! Adds a shape.
    void AddShape_(const ShapePtr &shape);

    friend class Reader_;
};

}  // namespace Graph
