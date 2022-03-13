#pragma once

#include <string>
#include <unordered_map>

#include "Math/Types.h"
#include "SG/Texture.h"

namespace Parser { class Registry; }

namespace SG {

/// The TextureAtlas class is derived from Texture and adds a field with
/// Uniforms and API to access individual images from the atlas.
class TextureAtlas : public Texture {
  public:
    /// Looks up the named image in the texture atlas and sets the scale and
    /// offset parameters to the texture scale and offset for that image within
    /// the atlas. Asserts if it is not found.
    void FindImageTransform(const std::string &name,
                            Vector2f &scale, Vector2f &offset) const;

  protected:
    TextureAtlas() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

  private:
    /// Type of map storing image transforms.
    typedef std::unordered_map<std::string, Vector4f> ImageMap_;

    /// \name Parsed Fields
    ///@{
    Parser::ObjectListField<Uniform> uniforms_{"uniforms"};
    ///@}

    /// Maps name of an image within the atlas to a Vector4f containing the
    /// scale and offset values.
    ImageMap_ image_map_;

    friend class Parser::Registry;
};

}  // namespace SG
