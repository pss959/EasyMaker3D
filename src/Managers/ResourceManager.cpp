#include "Managers/ResourceManager.h"

#include <filesystem>

#include "Loader.h"

// XXXX Check file times, store in maps, etc.

ResourceManager::ResourceManager() :
    shader_manager_(new ion::gfxutils::ShaderManager) {
}

ResourceManager::~ResourceManager() {
}

ion::gfxutils::ShaderManager & ResourceManager::GetShaderManager() {
    return *shader_manager_;
}

ion::gfx::NodePtr ResourceManager::LoadNode(const std::string &path) {
    return Loader(*this).LoadNode(GetPath_("nodes", path));
}

ion::gfx::ImagePtr ResourceManager::LoadTextureImage(const std::string &path) {
    return Loader(*this).LoadImage(GetPath_("textures", path));
}

std::string ResourceManager::LoadShaderSource(const std::string &path) {
    return Loader(*this).LoadFile(GetPath_("shaders", path));
}

std::string ResourceManager::GetPath_(const std::string &type_name,
                                      const std::string &path) {
    std::filesystem::path full_path(RESOURCE_DIR);
    full_path /= type_name;
    full_path /= path;
    return full_path.native();
}
