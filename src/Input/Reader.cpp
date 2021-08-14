#include "Input/Reader.h"

#include "Input/Exception.h"
#include "Input/Extractor.h"
#include "Input/ParserSpecs.h"
#include "Input/Tracker.h"
#include "Parser/Parser.h"
#include "Parser/Visitor.h"

namespace Input {

Reader::Reader(Tracker &tracker) : tracker_(tracker) {
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
        scene = Extractor(tracker_).ExtractScene(*root);
    }
    // XXXX
    return scene;
}

}  // namespace Input
