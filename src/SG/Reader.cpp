#include "SG/Reader.h"

#include "Assert.h"
#include "Parser/Parser.h"
#include "SG/Box.h"
#include "SG/Camera.h"
#include "SG/Cylinder.h"
#include "SG/Ellipsoid.h"
#include "SG/Image.h"
#include "SG/ImportedShape.h"
#include "SG/LayoutOptions.h"
#include "SG/LightingPass.h"
#include "SG/Line.h"
#include "SG/Node.h"
#include "SG/PointLight.h"
#include "SG/Polygon.h"
#include "SG/Reader.h"
#include "SG/Rectangle.h"
#include "SG/RenderPass.h"
#include "SG/Sampler.h"
#include "SG/Scene.h"
#include "SG/ShaderProgram.h"
#include "SG/ShaderSource.h"
#include "SG/ShadowPass.h"
#include "SG/Shape.h"
#include "SG/StateTable.h"
#include "SG/TextNode.h"
#include "SG/Texture.h"
#include "SG/Tracker.h"
#include "SG/Typedefs.h"
#include "SG/Uniform.h"
#include "SG/UniformDef.h"
#include "Util/General.h"

namespace SG {

Reader::Reader(Tracker &tracker,
               const ion::gfxutils::ShaderManagerPtr &shader_manager,
               const ion::text::FontManagerPtr &font_manager) :
    tracker_(tracker),
    shader_manager_(shader_manager),
    font_manager_(font_manager) {
}

Reader::~Reader() {
}

ScenePtr Reader::ReadScene(const Util::FilePath &path, bool set_up_ion) {
    // Use a Parser to read the scene.
    Parser::Parser parser;

    // Register all SG object types.
    RegisterTypes_(parser);

    Parser::ObjectPtr root = parser.ParseFile(path);

    // Tell the Tracker about included file dependencies.
    for (const auto &dep: parser.GetDependencies())
        tracker_.AddDependency(dep.including_path, dep.included_path);

    ScenePtr scene = Util::CastToDerived<Parser::Object, Scene>(root);
    ASSERT(scene);
    scene->SetPath(path);

    if (set_up_ion) {
        SG::Object::IonContext context(tracker_, shader_manager_,
                                       font_manager_);
        scene->SetUpIon(context);
    }

    return scene;
}

#define ADD_TYPE_(TYPE) parser.RegisterObjectType(TYPE::GetObjectSpec())

void Reader::RegisterTypes_(Parser::Parser &parser) {
    // Add all concrete SG types so they can be parsed.
    ADD_TYPE_(Box);
    ADD_TYPE_(Camera);
    ADD_TYPE_(Cylinder);
    ADD_TYPE_(Ellipsoid);
    ADD_TYPE_(Image);
    ADD_TYPE_(ImportedShape);
    ADD_TYPE_(LayoutOptions);
    ADD_TYPE_(LightingPass);
    ADD_TYPE_(Line);
    ADD_TYPE_(Node);
    ADD_TYPE_(PointLight);
    ADD_TYPE_(Polygon);
    ADD_TYPE_(Rectangle);
    ADD_TYPE_(RenderPass);
    ADD_TYPE_(Sampler);
    ADD_TYPE_(Scene);
    ADD_TYPE_(ShaderProgram);
    ADD_TYPE_(ShaderSource);
    ADD_TYPE_(ShadowPass);
    ADD_TYPE_(StateTable);
    ADD_TYPE_(TextNode);
    ADD_TYPE_(Texture);
    ADD_TYPE_(Uniform);
    ADD_TYPE_(UniformDef);
}

#undef ADD_TYPE_

}  // namespace SG
