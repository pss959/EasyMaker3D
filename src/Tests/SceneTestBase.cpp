#include "Tests/SceneTestBase.h"

#include "Parser/Parser.h"
#include "SG/FileMap.h"
#include "Util/Assert.h"
#include "Util/Tuning.h"

const float SceneTestBase::MS = TK::kInitialModelScale;

std::string SceneTestBase::BuildSceneString(const std::string &contents) {
    const std::string header = R"(
Scene {
  render_passes: [
    LightingPass "Lighting" {
      shader_programs: [<"programs/BaseColor.emd">]
    }
  ],
  root_node: Node "Root" {
)";
    return header + contents + "}}";
}

SG::ScenePtr SceneTestBase::ReadScene(const std::string &input,
                                      bool set_up_ion) {
    if (! ion_context_)
        InitIonContext_();

    TempFile file(input);
    auto scene = reader.ReadScene(file.GetPath(), ion_context_->GetFileMap());
    if (scene && set_up_ion)
        scene->SetUpIon(ion_context_);
    return scene;
}

void SceneTestBase::InitIonContext_() {
    ASSERT(! ion_context_);
    ion_context_.reset(new SG::IonContext);
    ion_context_->SetFileMap(SG::FileMapPtr(new SG::FileMap()));
    ion_context_->SetShaderManager(
        ion::gfxutils::ShaderManagerPtr(new ion::gfxutils::ShaderManager));
    ion_context_->SetFontManager(
        ion::text::FontManagerPtr(new ion::text::FontManager));
}

Parser::ObjectPtr SceneTestBase::ReadItem_(const std::string &input) {
    TempFile file(input);
    Parser::Parser parser;
    return parser.ParseFromString(input);
}
