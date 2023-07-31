#include "SG/Reader.h"

#include "Parser/Parser.h"
#include "SG/FileMap.h"
#include "SG/Scene.h"
#include "Util/Assert.h"

namespace SG {

ScenePtr Reader::ReadScene(const FilePath &path, FileMap &file_map) {
    // Use a Parser to read the scene.
    Parser::Parser parser;

    Parser::ObjectPtr root = parser.ParseFile(path);

    // Tell the FileMap about included file dependencies.
    for (const auto &dep: parser.GetDependencies())
        file_map.AddDependency(dep.including_path, dep.included_path);

    ScenePtr scene = std::dynamic_pointer_cast<Scene>(root);
    ASSERT(scene);
    scene->SetPath(path);

    return scene;
}

}  // namespace SG
