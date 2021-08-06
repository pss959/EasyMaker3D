#include "Loader.h"

#include <ion/gfx/shaderinputregistry.h>
#include <ion/math/angle.h>
#include <ion/math/matrix.h>
#include <ion/math/rotation.h>
#include <ion/math/transformutils.h>
#include <ion/math/vector.h>

#include <ion/gfxutils/printer.h> // XXXX

using ion::gfx::NodePtr;
using ion::gfx::ShapePtr;
using ion::math::Anglef;
using ion::math::Matrix4f;
using ion::math::Rotationf;
using ion::math::Vector3f;
using ion::math::Vector4f;

NodePtr Loader::LoadNode(const std::string &path) {
    return ExtractNode_(*ParseFile_(FullPath(path)));
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

NodePtr Loader::ExtractNode_(const Parser::Object &obj) {
    NodePtr node(new ion::gfx::Node);

    // Verify that this is a Node.
    if (obj.type_name != "Node")
        ThrowTypeMismatch_(obj, "Node");

    const ion::gfx::ShaderInputRegistryPtr& global_reg =
        ion::gfx::ShaderInputRegistry::GetGlobalRegistry();

    // Matrix stuff.
    bool      need_matrix = false;
    Vector3f  scale(1, 1, 1);
    Rotationf rot;
    Vector3f  trans = Vector3f::Zero();

    // Validate the fields.
    for (const Parser::Field &field: obj.fields) {
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
        else if (field.name == "shapes") {
            for (const Parser::ObjectPtr &shape_obj: field.objects_val)
                node->AddShape(ExtractShape_(*shape_obj));
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

ShapePtr Loader::ExtractShape_(const Parser::Object &obj) {
    ShapePtr shape(new ion::gfx::Shape);

    // Verify that this is a Shape of some type.
    if (obj.type_name == "Cylinder") {
    }
    else if (obj.type_name == "Sphere") {
        // ...
    }
    else {
        ThrowTypeMismatch_(obj, "some sort of Shape");
    }
    return shape;
}

void Loader::ThrowTypeMismatch_(const Parser::Object &obj,
                                const std::string &expected_type) {
    throw Exception(obj.path, obj.line_number,
                    "Expected " + expected_type +
                    " object, got " + obj.type_name);
}
