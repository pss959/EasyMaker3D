#include "App/SceneLoader.h"

#include <exception>

#include "IO/Reader.h"

SceneLoader::SceneLoader() :
    file_map_(new SG::FileMap),
    shader_manager_(new ion::gfxutils::ShaderManager),
    font_manager_(new ion::text::FontManager),
    ion_context_(new SG::IonContext) {

    ion_context_->SetFileMap(file_map_);
    ion_context_->SetShaderManager(shader_manager_);
    ion_context_->SetFontManager(font_manager_);
}

SG::ScenePtr SceneLoader::LoadScene(const FilePath &path) {
    // Wipe out all previous shaders to avoid conflicts.
    shader_manager_.Reset(new ion::gfxutils::ShaderManager);
    ion_context_->Reset();
    ion_context_->SetShaderManager(shader_manager_);

    try {
        Reader reader;
        scene_ = reader.ReadScene(path, *file_map_);
        scene_->SetUpIon(ion_context_);
    }
    catch (std::exception &ex) {
        std::cerr << "*** Caught exception loading scene:\n"
                  << ex.what() << "\n";
        scene_.reset();
    }
    return scene_;
}
