#include "Reader.h"

#include "Assert.h"
#include "Parser/Parser.h"
#include "SG/Init.h"
#include "SG/Scene.h"
#include "SG/Tracker.h"
#include "Util/General.h"

Reader::Reader(SG::Tracker &tracker,
               const ion::gfxutils::ShaderManagerPtr &shader_manager,
               const ion::text::FontManagerPtr &font_manager) :
    tracker_(tracker),
    shader_manager_(shader_manager),
    font_manager_(font_manager) {
}

Reader::~Reader() {
}

SG::ScenePtr Reader::ReadScene(const Util::FilePath &path, bool set_up_ion) {
    // Use a Parser to read the scene.
    Parser::Parser parser;

    // Register all SG object types.
    SG::RegisterTypes(parser);

    Parser::ObjectPtr root = parser.ParseFile(path);

    // Tell the Tracker about included file dependencies.
    for (const auto &dep: parser.GetDependencies())
        tracker_.AddDependency(dep.including_path, dep.included_path);

    SG::ScenePtr scene = Util::CastToDerived<SG::Scene>(root);
    ASSERT(scene);
    scene->SetPath(path);

    if (set_up_ion) {
        SG::Object::IonContext context(tracker_, shader_manager_,
                                       font_manager_);
        scene->SetUpIon(context);
    }

    return scene;
}
