#include "Loader.h"

#include <ion/gfx/shaderinputregistry.h>
#include <ion/math/angle.h>
#include <ion/math/matrix.h>
#include <ion/math/rotation.h>
#include <ion/math/transformutils.h>
#include <ion/math/vector.h>

#include <ion/gfxutils/printer.h> // XXXX

using ion::gfx::NodePtr;
using ion::math::Anglef;
using ion::math::Matrix4f;
using ion::math::Rotationf;
using ion::math::Vector3f;
using ion::math::Vector4f;

NodePtr Loader::LoadNode(const std::string &path) {
    std::string full_path = FullPath(path);
    Parser::ObjectPtr root = ParseFile_(full_path);

    NodePtr node(new ion::gfx::Node);

    // Verify that this is a Node.
    ExpectType_(*root, "Node");

    const ion::gfx::ShaderInputRegistryPtr& global_reg =
        ion::gfx::ShaderInputRegistry::GetGlobalRegistry();

    // Matrix stuff.
    bool      need_matrix = false;
    Vector3f  scale(1, 1, 1);
    Rotationf rot;
    Vector3f  trans = Vector3f::Zero();

    // Validate the fields.
    for (const Parser::Field &field: root->fields) {
        if (field.name == "name")
            node->SetLabel(field.string_val);
        if (field.name == "scale") {
            scale = field.vector3_val;
            need_matrix = true;
        }
        else if (field.name == "rotation") {
            Vector4f v = field.vector4_val;
            rot = Rotationf::FromAxisAndAngle(Vector3f(v[0], v[1], v[2]),
                                              Anglef::FromDegrees(v[3]));
            need_matrix = true;
        }
        else if (field.name == "translation") {
            trans = field.vector3_val;
            need_matrix = true;
        }
    }
    if (need_matrix) {
        Matrix4f mat =
            ion::math::TranslationMatrix(trans) *
            ion::math::RotationMatrixH(rot) *
            ion::math::ScaleMatrixH(scale);
        node->AddUniform(
            global_reg->Create<ion::gfx::Uniform>("uModelviewMatrix", mat));
    }

    // XXXX More...

    // XXXX DEBUGGING:
    ion::gfxutils::Printer printer;
    printer.EnableAddressPrinting(false);
    printer.PrintScene(node, std::cout);

    return node;
}

Parser::ObjectPtr Loader::ParseFile_(const std::string &path) {
    std::shared_ptr<Parser::Object> root;
    try {
        Parser parser;
        root = parser.ParseFile(path);
    }
    catch (Parser::Exception &ex) {
        throw Exception(path, std::string("Failed parsing:\n") + ex.what());
    }
    return root;
}

void Loader::ExpectType_(const Parser::Object &obj,
                         const std::string &type_name) {
    if (obj.type_name != type_name) {
        throw Exception(
            obj.path, obj.line_number,
            "Expected " + type_name + "object, got " + obj.type_name);
    }
}
