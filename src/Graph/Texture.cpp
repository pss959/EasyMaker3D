#include "Graph/Texture.h"

#include "Graph/Image.h"

namespace Graph {

Texture::Texture() : i_texture_(new ion::gfx::Texture) {
}

void Texture::SetName_(const std::string &name) {
    Object::SetName_(name);
    i_texture_->SetLabel(name);
}

void Texture::SetUniformName_(const std::string &name) {
    uniform_name_ = name;
}

void Texture::SetImage_(const ImagePtr &image) {
    image_ = image;
    i_texture_->SetImage(0U, image->GetIonImage());
}

}  // namespace Graph
