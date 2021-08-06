#include "Loader.h"

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfxutils/shapeutils.h>
#include <ion/math/angle.h>
#include <ion/math/matrix.h>
#include <ion/math/rotation.h>
#include <ion/math/transformutils.h>
#include <ion/math/vector.h>

#include <ion/gfxutils/printer.h> // XXXX

#include "Util.h"

using ion::gfx::NodePtr;
using ion::gfx::ShapePtr;
using ion::math::Anglef;
using ion::math::Matrix4f;
using ion::math::Rotationf;
using ion::math::Vector3f;
using ion::math::Vector4f;

// ----------------------------------------------------------------------------
// Field type maps.
// ----------------------------------------------------------------------------

//! Shorthand macro.
#define TYPE_ENTRY_(name, type) { name, Parser::Field::Type::type }

static const Parser::FieldTypeMap node_field_type_map_{
    TYPE_ENTRY_("name",              kString),
    TYPE_ENTRY_("rotation",          kVector4),
    TYPE_ENTRY_("scale",             kVector3),
    TYPE_ENTRY_("translation",       kVector3),
    TYPE_ENTRY_("shapes",            kObjects),
    TYPE_ENTRY_("children",          kObjects),
};

static const Parser::FieldTypeMap box_field_type_map_{
    TYPE_ENTRY_("name",              kString),
    TYPE_ENTRY_("size",              kVector3),
};

static const Parser::FieldTypeMap cylinder_field_type_map_{
    TYPE_ENTRY_("name",              kString),
    TYPE_ENTRY_("bottom_radius",     kScalar),
    TYPE_ENTRY_("top_radius",        kScalar),
    TYPE_ENTRY_("height",            kScalar),
    TYPE_ENTRY_("has_top_cap",       kBool),
    TYPE_ENTRY_("has_bottom_cap",    kBool),
    TYPE_ENTRY_("shaft_band_count",  kInteger),
    TYPE_ENTRY_("cap_band_count",    kInteger),
    TYPE_ENTRY_("sector_count",      kInteger),
};

static const Parser::FieldTypeMap ellipsoid_field_type_map_{
    TYPE_ENTRY_("name",              kString),
    TYPE_ENTRY_("longitude_start",   kScalar),
    TYPE_ENTRY_("longitude_end",     kScalar),
    TYPE_ENTRY_("latitude_start",    kScalar),
    TYPE_ENTRY_("latitude_end",      kScalar),
    TYPE_ENTRY_("band_count",        kInteger),
    TYPE_ENTRY_("sector_count",      kInteger),
    TYPE_ENTRY_("size",              kVector3),
};

#undef TYPE_ENTRY_

// ----------------------------------------------------------------------------
// Loader implementation.
// ----------------------------------------------------------------------------

NodePtr Loader::LoadNode(const std::string &path) {
    return ExtractNode_(*ParseFile_(FullPath(path)));
}

Parser::ObjectPtr Loader::ParseFile_(const std::string &path) {
    // Merge all of the type maps into one.
    Parser::FieldTypeMap field_type_map;
    field_type_map.insert(node_field_type_map_.begin(),
                          node_field_type_map_.end());
    field_type_map.insert(box_field_type_map_.begin(),
                          box_field_type_map_.end());
    field_type_map.insert(cylinder_field_type_map_.begin(),
                          cylinder_field_type_map_.end());
    field_type_map.insert(ellipsoid_field_type_map_.begin(),
                          ellipsoid_field_type_map_.end());

    std::shared_ptr<Parser::Object> root;
    try {
        Parser parser;
        root = parser.ParseFile(path, field_type_map);
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
        else if (field.name == "scale") {
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
        else if (field.name == "children") {
            for (const Parser::ObjectPtr &child_obj: field.objects_val)
                node->AddChild(ExtractNode_(*child_obj));
        }
        else {
            ThrowBadField_(obj, field);
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

#define CHECK_SPEC_FIELD_(field_name, val) \
    if (field.name == #field_name) spec.field_name = field.val
#define CHECK_SPEC_ANGLE_(field_name) \
    if (field.name == #field_name) \
        spec.field_name = Anglef::FromDegrees(field.scalar_val)

ShapePtr Loader::ExtractShape_(const Parser::Object &obj) {
    ShapePtr shape;

    std::string label;

    // Verify that this is a Shape of some type.
    if (obj.type_name == "Box") {
        ion::gfxutils::BoxSpec spec;
        spec.vertex_type = ion::gfxutils::ShapeSpec::kPosition;
        for (const Parser::Field &field: obj.fields) {
            if (field.name == "name")
                label = field.string_val;
            else CHECK_SPEC_FIELD_(size, vector3_val);
            else
                ThrowBadField_(obj, field);
        }
        shape = ion::gfxutils::BuildBoxShape(spec);
    }
    else if (obj.type_name == "Cylinder") {
        ion::gfxutils::CylinderSpec spec;
        spec.vertex_type = ion::gfxutils::ShapeSpec::kPosition;
        for (const Parser::Field &field: obj.fields) {
            if (field.name == "name")
                label = field.string_val;
            else CHECK_SPEC_FIELD_(bottom_radius,    scalar_val);
            else CHECK_SPEC_FIELD_(top_radius,       scalar_val);
            else CHECK_SPEC_FIELD_(height,           scalar_val);
            else CHECK_SPEC_FIELD_(has_top_cap,      bool_val);
            else CHECK_SPEC_FIELD_(has_bottom_cap,   bool_val);
            else CHECK_SPEC_FIELD_(shaft_band_count, integer_val);
            else CHECK_SPEC_FIELD_(cap_band_count,   integer_val);
            else CHECK_SPEC_FIELD_(sector_count,     integer_val);
            else
                ThrowBadField_(obj, field);
        }
        shape = ion::gfxutils::BuildCylinderShape(spec);
    }
    else if (obj.type_name == "Ellipsoid") {
        ion::gfxutils::EllipsoidSpec spec;
        spec.vertex_type = ion::gfxutils::ShapeSpec::kPosition;
        for (const Parser::Field &field: obj.fields) {
            if (field.name == "name")
                label = field.string_val;
            else CHECK_SPEC_ANGLE_(longitude_start);
            else CHECK_SPEC_ANGLE_(longitude_end);
            else CHECK_SPEC_ANGLE_(latitude_start);
            else CHECK_SPEC_ANGLE_(latitude_end);
            else CHECK_SPEC_FIELD_(band_count,   integer_val);
            else CHECK_SPEC_FIELD_(sector_count, integer_val);
            else CHECK_SPEC_FIELD_(size,         vector3_val);
            else
                ThrowBadField_(obj, field);
        }
        shape = ion::gfxutils::BuildEllipsoidShape(spec);
    }
    else {
        throw Exception(obj.path, obj.line_number,
                        "Unknown shape type '" + obj.type_name + "'");
    }

    shape->SetLabel(label);
    return shape;
}

#undef CHECK_SPEC_FIELD_
#undef CHECK_SPEC_ANGLE_

void Loader::ThrowTypeMismatch_(const Parser::Object &obj,
                                const std::string &expected_type) {
    throw Exception(obj.path, obj.line_number,
                    "Expected " + expected_type +
                    " object, got " + obj.type_name);
}

void Loader::ThrowBadField_(const Parser::Object &obj,
                            const Parser::Field &field) {
    throw Exception(obj.path, obj.line_number,
                    "Invalid field '" + field.name +
                    "' found in " + obj.type_name + " object");
}
