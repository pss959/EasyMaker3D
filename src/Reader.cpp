#include "Reader.h"

#include "Assert.h"
#include "Parser/Parser.h"
#include "SG/Scene.h"
#include "SG/Tracker.h"
#include "Util/General.h"

// These define all the classes that are registered with the Parser.
#include "SG/Init.h"
#include "Models/Init.h"
#include "Widgets/Init.h"

SG::ScenePtr Reader::ReadScene(const Util::FilePath &path,
                               SG::Tracker &tracker) {
    // Use a Parser to read the scene.
    Parser::Parser parser;

    // Register all known object types.
    SG::RegisterTypes(parser);
    RegisterModelTypes(parser);
    RegisterWidgetTypes(parser);

    Parser::ObjectPtr root = parser.ParseFile(path);

    // Tell the Tracker about included file dependencies.
    for (const auto &dep: parser.GetDependencies())
        tracker.AddDependency(dep.including_path, dep.included_path);

    SG::ScenePtr scene = Util::CastToDerived<SG::Scene>(root);
    ASSERT(scene);
    scene->SetPath(path);

    return scene;
}
