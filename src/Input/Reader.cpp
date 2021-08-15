#include "Input/Reader.h"

#include "Input/Exception.h"
#include "Input/Extractor.h"
#include "Input/ParserSpecs.h"
#include "Input/Tracker.h"
#include "Parser/Parser.h"
#include "Parser/Visitor.h"

using ion::gfxutils::ShaderManager;

namespace Input {

Reader::Reader(Tracker &tracker, ShaderManager &shader_manager) :
    tracker_(tracker), shader_manager_(shader_manager) {
}

Reader::~Reader() {
}

Graph::ScenePtr Reader::ReadScene(const Util::FilePath &path) {
    // Use a Parser to read the scene.
    Parser::ObjectPtr root;
    try {
        Parser::Parser parser(node_specs_);
        root = parser.ParseFile(path);
    }
    catch (Parser::Exception &ex) {
        throw Exception(path, std::string("Parsing failed:\n") + ex.what());
    }

    Graph::ScenePtr scene;
    if (root.get()) {
        // Tell the Tracker about included file dependencies.
        Parser::Visitor::ObjectFunc func = [this](const Parser::Object &obj){
            for (const std::string &p: obj.included_paths)
                this->tracker_.AddDependency(obj.path, p);
        };
        Parser::Visitor::VisitObjects(*root, func);

        // Extract from root.
        scene = Extractor(tracker_, shader_manager_).ExtractScene(*root);
    }
    return scene;
}

}  // namespace Input
