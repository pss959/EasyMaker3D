#include "IO/Reader.h"

#include "Parser/Parser.h"
#include "SG/Scene.h"
#include "SG/Tracker.h"
#include "Util/Assert.h"
#include "Util/General.h"

SG::ScenePtr Reader::ReadScene(const FilePath &path, SG::Tracker &tracker) {
    // Use a Parser to read the scene.
    Parser::Parser parser;

    Parser::ObjectPtr root = parser.ParseFile(path);

    // Tell the Tracker about included file dependencies.
    for (const auto &dep: parser.GetDependencies())
        tracker.AddDependency(dep.including_path, dep.included_path);

    SG::ScenePtr scene = Util::CastToDerived<SG::Scene>(root);
    ASSERT(scene);
    scene->SetPath(path);

    return scene;
}
