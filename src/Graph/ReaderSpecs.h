// ----------------------------------------------------------------------------
// Parser::ObjectSpec specifications for loaded types.
// ----------------------------------------------------------------------------

//! Shorthand macro.
#define FIELD_(NAME, COUNT, TYPE) { NAME, Parser::ValueType::TYPE, COUNT }

static const std::vector<Parser::ObjectSpec> node_specs_{
    { "Scene",
      { FIELD_("camera",           1, kObject),
        FIELD_("nodes",            1, kObjectList), }
    },
    { "Camera",
      { FIELD_("position",         3, kFloat),
        FIELD_("orientation",      4, kFloat),
        FIELD_("fov",              1, kFloat),
        FIELD_("near",             1, kFloat),
        FIELD_("far",              1, kFloat), }
    },
    { "Node",
      { FIELD_("enabled",          1, kBool),
        FIELD_("scale",            3, kFloat),
        FIELD_("rotation",         4, kFloat),
        FIELD_("translation",      3, kFloat),
        FIELD_("state_table",      1, kObject),
        FIELD_("shader",           1, kObject),
        FIELD_("uniforms",         1, kObjectList),
        FIELD_("shapes",           1, kObjectList),
        FIELD_("children",         1, kObjectList), }
    },
    { "StateTable",
      { FIELD_("clear_color",      4, kFloat),
        FIELD_("enable_cap",       1, kString),
        FIELD_("disable_cap",      1, kString), }
    },
    { "Shader",
      { FIELD_("uniform_defs",     1, kObjectList),
        FIELD_("vertex_program",   1, kString),
        FIELD_("geometry_program", 1, kString),
        FIELD_("fragment_program", 1, kString), }
    },
    { "UniformDef",
      { FIELD_("type",             1, kString), }
    },
    { "Uniform",
      { FIELD_("float_val",        1, kFloat),
        FIELD_("int_val",          1, kInteger),
        FIELD_("uint_val",         1, kUInteger),
        FIELD_("vec2f_val",        2, kFloat),
        FIELD_("vec3f_val",        3, kFloat),
        FIELD_("vec4f_val",        4, kFloat),
        FIELD_("vec2i_val",        2, kInteger),
        FIELD_("vec3i_val",        3, kInteger),
        FIELD_("vec4i_val",        4, kInteger),
        FIELD_("vec2ui_val",       2, kUInteger),
        FIELD_("vec3ui_val",       3, kUInteger),
        FIELD_("vec4ui_val",       4, kUInteger),
        FIELD_("mat2_val",         4, kFloat),
        FIELD_("mat3_val",         9, kFloat),
        FIELD_("mat4_val",        16, kFloat),
        FIELD_("texture_val",      1, kObject),
      },
    },
    { "Texture",
      { FIELD_("image_file",       1, kString),
        FIELD_("sampler",          1, kObject), }
    },
    { "Sampler",
      { FIELD_("wrap_s_mode",      1, kString),
        FIELD_("wrap_t_mode",      1, kString), }
    },

    // Shapes:
    { "Box",
      { FIELD_("size",             3, kFloat), }
    },
    { "Cylinder",
      { FIELD_("bottom_radius",    1, kFloat),
        FIELD_("top_radius",       1, kFloat),
        FIELD_("height",           1, kFloat),
        FIELD_("has_top_cap",      1, kBool),
        FIELD_("has_bottom_cap",   1, kBool),
        FIELD_("shaft_band_count", 1, kInteger),
        FIELD_("cap_band_count",   1, kInteger),
        FIELD_("sector_count",     1, kInteger), }
    },
    { "Ellipsoid",
      { FIELD_("longitude_start",  1, kFloat),
        FIELD_("longitude_end",    1, kFloat),
        FIELD_("latitude_start",   1, kFloat),
        FIELD_("latitude_end",     1, kFloat),
        FIELD_("band_count",       1, kInteger),
        FIELD_("sector_count",     1, kInteger),
        FIELD_("size",             3, kFloat), }
    },
    { "Polygon",
      { FIELD_("sides",            1, kInteger),
        FIELD_("plane_normal",     1, kString), }
    },
    { "Rectangle",
      { FIELD_("plane_normal",     1, kString),
        FIELD_("size",             2, kFloat), }
    },
};

#undef FIELD_
