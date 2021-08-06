#include "Loader.h"

#include "Parser.h"

using ion::gfx::NodePtr;

#if XXXX_JSON
#include <jsoncpp/json/json.h>
NodePtr Loader::LoadNode(const std::string &path) {
    std::ifstream in(FullPath(path));
    if (in.fail())
        throw Exception(path, "Failed to open file");

    try {
        Json::Value root;
        in >> root;
    }
    catch (Json::Exception &ex) {
        throw Exception(path, std::string("Failed reading JSON: ") + ex.what());
    }
    NodePtr node;
    return node;
}
#endif

NodePtr Loader::LoadNode(const std::string &path) {
    std::shared_ptr<Parser::Object> root;
    try {
        Parser parser;
        root = parser.ParseFile(FullPath(path));
    }
    catch (Parser::Exception &ex) {
        throw Exception(path, std::string("Failed parsing: ") + ex.what());
    }

    // XXXX Convert root to Node.
    NodePtr node;
    return node;
}
