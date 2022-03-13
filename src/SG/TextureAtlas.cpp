#include "SG/TextureAtlas.h"

#include "SG/Uniform.h"
#include "Util/Assert.h"

namespace SG {

void TextureAtlas::AddFields() {
    AddField(uniforms_);
    Texture::AddFields();
}

void TextureAtlas::CreationDone() {
    for (const auto &u: uniforms_.GetValue()) {
        ASSERT(! u->GetName().empty());
        ASSERT(u->GetLastFieldSet() == "vec4f_val");
        image_map_[u->GetName()] = u->GetVector4f();
    }
}

void TextureAtlas::FindImageTransform(const std::string &name,
                                      Vector2f &scale, Vector2f &offset) const {
    const auto it = image_map_.find(name);
    ASSERTM(it != image_map_.end(), "Image '" + name + "'");
    const Vector4f &v = it->second;
    scale.Set(v[0], v[1]);
    offset.Set(v[2], v[3]);
}

}  // namespace SG
