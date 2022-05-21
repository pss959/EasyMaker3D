#include "SceneTestBase.h"

#include "App/RegisterTypes.h"
#include "Managers/ColorManager.h"
#include "SG/Tracker.h"

const std::string SceneTestBase::str1 =
    "Scene {\n"
        " render_passes: [ LightingPass \"Lighting\" {\n"
        "   shader_programs: [<\"programs/BaseColor.mvn\">],"
        " }],\n"
    " root_node: Node \"Root\" {";

const std::string SceneTestBase::str2 = "}}";

SceneTestBase::SceneTestBase() {
    RegisterTypes();
}

SceneTestBase::~SceneTestBase() {
    // Clear the registry and managers so the next test has a fresh start.
    UnregisterTypes();
    ColorManager::ClearSpecialColors();
}

SG::ScenePtr SceneTestBase::ReadScene(const std::string &input) {
    TempFile file(input);

    // Create new instances to avoid inter-test pollution.
    ion_context.reset(new SG::IonContext);
    ion_context->SetTracker(SG::TrackerPtr(new SG::Tracker()));
    ion_context->SetShaderManager(
        ion::gfxutils::ShaderManagerPtr(new ion::gfxutils::ShaderManager));
    ion_context->SetFontManager(
        ion::text::FontManagerPtr(new ion::text::FontManager));

    SG::ScenePtr scene = reader.ReadScene(file.GetPath(),
                                          ion_context->GetTracker());
    if (scene && set_up_ion)
        scene->SetUpIon(ion_context);
    return scene;
}
