#include "SG/Reader.h"

#include "NParser/Parser.h"
#include "SG/Box.h"
#include "SG/Camera.h"
#include "SG/Cylinder.h"
#include "SG/Ellipsoid.h"
#include "SG/Image.h"
#include "SG/Node.h"
#include "SG/Polygon.h"
#include "SG/Reader.h"
#include "SG/Rectangle.h"
#include "SG/Sampler.h"
#include "SG/Scene.h"
#include "SG/ShaderProgram.h"
#include "SG/ShaderSource.h"
#include "SG/Shape.h"
#include "SG/StateTable.h"
#include "SG/Texture.h"
#include "SG/Tracker.h"
#include "SG/Typedefs.h"
#include "SG/Uniform.h"
#include "SG/UniformDef.h"
#include "Util/General.h"

using ion::gfxutils::ShaderManager;

namespace SG {

Reader::Reader(Tracker &tracker, ShaderManager &shader_manager) :
    tracker_(tracker), shader_manager_(shader_manager) {
}

Reader::~Reader() {
}

ScenePtr Reader::ReadScene(const Util::FilePath &path) {
    // Use a Parser to read the scene.
    NParser::Parser parser;

    // Register all SG object types.
    RegisterTypes_(parser);

    NParser::ObjectPtr root = parser.ParseFile(path);

    // Tell the Tracker about included file dependencies.
    for (const auto &dep: parser.GetDependencies())
        tracker_.AddDependency(dep.including_path, dep.included_path);

    ScenePtr scene = Util::CastToDerived<NParser::Object, Scene>(root);
    assert(scene);

    return scene;
}

#define ADD_TYPE_(TYPE)                                                 \
    parser.RegisterObjectType(#TYPE, TYPE::GetFieldSpecs(),             \
                              []{ return new TYPE; })

void Reader::RegisterTypes_(NParser::Parser &parser) {
    // Add all concrete SG types so they can be parsed.
    ADD_TYPE_(Box);
    ADD_TYPE_(Camera);
    ADD_TYPE_(Cylinder);
    ADD_TYPE_(Ellipsoid);
    ADD_TYPE_(Image);
    ADD_TYPE_(Node);
    ADD_TYPE_(Polygon);
    ADD_TYPE_(Rectangle);
    ADD_TYPE_(Sampler);
    ADD_TYPE_(Scene);
    ADD_TYPE_(ShaderProgram);
    ADD_TYPE_(ShaderSource);
    ADD_TYPE_(StateTable);
    ADD_TYPE_(Texture);
    ADD_TYPE_(Uniform);
    ADD_TYPE_(UniformDef);

    // XXXX Need to deal with tracker in some of these.
}

#undef ADD_TYPE_

}  // namespace SG
