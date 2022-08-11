#include "SceneTestBase.h"

#include "App/RegisterTypes.h"
#include "Base/Tuning.h"
#include "SG/FileMap.h"

const std::string SceneTestBase::str1 =
    "Scene {\n"
        " render_passes: [ LightingPass \"Lighting\" {\n"
        "   shader_programs: [<\"programs/BaseColor.mvn\">],"
        " }],\n"
    " root_node: Node \"Root\" {";

const std::string SceneTestBase::str2 = "}}";
const float       SceneTestBase::MS   = TK::kInitialModelScale;

SceneTestBase::SceneTestBase() {
    RegisterTypes();
}

SceneTestBase::~SceneTestBase() {
    // Clear the registry and managers so the next test has a fresh start.
    UnregisterTypes();
}

SG::ScenePtr SceneTestBase::ReadScene(const std::string &input) {
    TempFile file(input);

    // Create new instances to avoid inter-test pollution.
    ion_context.reset(new SG::IonContext);
    ion_context->SetFileMap(SG::FileMapPtr(new SG::FileMap()));
    ion_context->SetShaderManager(
        ion::gfxutils::ShaderManagerPtr(new ion::gfxutils::ShaderManager));
    ion_context->SetFontManager(
        ion::text::FontManagerPtr(new ion::text::FontManager));

    SG::ScenePtr scene = reader.ReadScene(file.GetPath(),
                                          ion_context->GetFileMap());
    if (scene && set_up_ion)
        scene->SetUpIon(ion_context);
    return scene;
}
