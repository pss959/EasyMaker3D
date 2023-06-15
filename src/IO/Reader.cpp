#include "IO/Reader.h"

#include "Parser/Parser.h"
#include "SG/FileMap.h"
#include "SG/Scene.h"
#include "Util/Assert.h"

SG::ScenePtr Reader::ReadScene(const FilePath &path, SG::FileMap &file_map) {
    // Use a Parser to read the scene.
    Parser::Parser parser;

    Parser::ObjectPtr root = parser.ParseFile(path);

    // Tell the FileMap about included file dependencies.
    for (const auto &dep: parser.GetDependencies())
        file_map.AddDependency(dep.including_path, dep.included_path);

    SG::ScenePtr scene = std::dynamic_pointer_cast<SG::Scene>(root);
    ASSERT(scene);
    scene->SetPath(path);

    return scene;
}
