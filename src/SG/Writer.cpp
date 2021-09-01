#include "SG/Reader.h"

#include <unordered_set>

#include "Assert.h"
#include "Parser/Parser.h"
#include "SG/Box.h"
#include "SG/Camera.h"
#include "SG/Cylinder.h"
#include "SG/Ellipsoid.h"
#include "SG/FileImage.h"
#include "SG/ImportedShape.h"
#include "SG/LayoutOptions.h"
#include "SG/Line.h"
#include "SG/Node.h"
#include "SG/PointLight.h"
#include "SG/Polygon.h"
#include "SG/ProceduralImage.h"
#include "SG/Rectangle.h"
#include "SG/RenderPass.h"
#include "SG/Sampler.h"
#include "SG/Scene.h"
#include "SG/ShaderProgram.h"
#include "SG/ShaderSource.h"
#include "SG/Shape.h"
#include "SG/StateTable.h"
#include "SG/TextNode.h"
#include "SG/Texture.h"
#include "SG/Tracker.h"
#include "SG/Typedefs.h"
#include "SG/Uniform.h"
#include "SG/UniformDef.h"
#include "SG/Writer.h"
#include "Util/Enum.h"
#include "Util/Flags.h"
#include "Util/General.h"

using ion::gfxutils::ShaderManager;

namespace SG {

// ----------------------------------------------------------------------------
// Helper Wrap_ class.
// ----------------------------------------------------------------------------

//! This wrapper class allows the << operator to be redefined for some Ion
//! classes to match what the Parser expects. This makes using the Reader+Writer
//! relatively idempotent.
template <typename T> struct Wrap_ {
    T val;
    Wrap_(const T &t) : val(t) {}
};

// Generic version just passes the wrapped item through.
template <typename T>
std::ostream & operator<<(std::ostream &out, const Wrap_<T> &w) {
    return out << w.val;
}

// Specialize for types we want to override.
template <int DIM, typename T>
std::ostream & operator<<(std::ostream &out,
                          const Wrap_<ion::math::Vector<DIM, T>> &w) {
    for (int i = 0; i < DIM; ++i) {
        if (i > 0)
            out << ' ';
        out << w.val[i];
    }
    return out;
}
template <int DIM, typename T>
std::ostream & operator<<(std::ostream &out,
                          const Wrap_<ion::math::Point<DIM, T>> &w) {
    for (int i = 0; i < DIM; ++i) {
        if (i > 0)
            out << ' ';
        out << w.val[i];
    }
    return out;
}

template <>
std::ostream & operator<<(std::ostream &out, const Wrap_<Anglef> &w) {
    return out << w.val.Degrees();
}

template <>
std::ostream & operator<<(std::ostream &out, const Wrap_<Rotationf> &w) {
    Vector3f axis;
    Anglef   angle;
    w.val.GetAxisAndAngle(&axis, &angle);
    return out << Wrap_(axis) << ' ' << Wrap_(angle);
}

// ----------------------------------------------------------------------------
// This internal class does most of the work.
// ----------------------------------------------------------------------------

class Writer_ {
  public:
    //! The constructor is passed the output stream.
    Writer_(std::ostream &out) : out_(out) {}

    void WriteScene(const Scene &scene);

  private:
    std::ostream    &out_;              //!< Stream passed to constructor.
    int              cur_depth_ = 0;    //!< Current depth in graph.
    bool             in_list_ = false;  //!< True when writing object list.

    //! Set storing named SG::Object instances that have been written
    //! already. This is used to detect instances.
    std::unordered_set<const Object *> written_named_objects_;

    static const int kIndent_   = 2;  //!< Spaces to indent each level.
    void WriteCamera_(const Camera &camera);
    void WritePointLight_(const PointLight &light);
    void WriteRenderPass_(const RenderPass &pass);
    void WriteNode_(const Node &node);
    void WriteStateTable_(const StateTable &table);
    void WriteShaderProgram_(const ShaderProgram &program);
    void WriteUniformDef_(const UniformDef &def);
    void WriteShaderSource_(const ShaderSource &src);
    void WriteTexture_(const Texture &tex);
    void WriteImage_(const Image &image);
    void WriteFileImage_(const FileImage &image);
    void WriteProceduralImage_(const ProceduralImage &image);
    void WriteSampler_(const Sampler &sampler);
    void WriteUniform_(const Uniform &uniform);
    void WriteShape_(const Shape &shape);
    void WriteBox_(const Box &box);
    void WriteCylinder_(const Cylinder &cyl);
    void WriteEllipsoid_(const Ellipsoid &ell);
    void WriteImportedShape_(const ImportedShape &imp);
    void WriteLine_(const Line &line);
    void WritePolygon_(const Polygon &poly);
    void WriteRectangle_(const Rectangle &rect);
    void WriteTextNode_(const TextNode &text);
    void WriteLayoutOptions_(const LayoutOptions &opts);

    template <typename T>
    void WriteField_(const std::string &name, const T &value) {
        WriteFieldName_(name);
        out_ << Wrap_(value) << ",\n";
    }

    template <typename E>
    void WriteEnumField_(const std::string &name, const E &value) {
        WriteFieldName_(name);
        out_ << '"' << Util::EnumName(value) << "\",\n";
    }

    template <typename E>
    void WriteFlagsField_(const std::string &name,
                          const Util::Flags<E> &value) {
        WriteFieldName_(name);
        out_ << '"' << value.ToString() << "\",\n";
    }

    template <typename T>
    void WriteObjField_(const std::string &name, const std::shared_ptr<T> &obj,
                        void (Writer_::* func)(const T &)) {
        if (obj) {
            if (! name.empty())
                WriteFieldName_(name);
            (this->*func)(*obj);
            out_ << ",\n";
        }
    }

    template <typename T>
    void WriteObjListField_(const std::string &name,
                            const std::vector<std::shared_ptr<T>> &list,
                            void (Writer_::* func)(const T &)) {
        if (! list.empty()) {
            ASSERT(! name.empty());
            in_list_ = true;
            WriteFieldName_(name);
            out_ << "[\n";
            ++cur_depth_;
            for (const auto &elt: list) {
                out_ << Indent_();
                (this->*func)(*elt);
                out_ << ",\n";
            }
            --cur_depth_;
            out_ << Indent_() << "],\n";
            in_list_ = false;
        }
    }

    void WriteObjListField_(const std::string &name,
                            const std::function<void()> &func);

    void WriteFieldName_(const std::string &name);

    //! Returns true if the object is an instance.
    bool WriteObjHeader_(const Object &obj);
    void WriteObjFooter_();

    std::string Indent_() { return std::string(kIndent_ * cur_depth_, ' '); }
};

// Specialize for bool.
template <>
void Writer_::WriteField_(const std::string &name, const bool &value) {
    WriteFieldName_(name);
    out_ << (value ? "True" : "False") << ",\n";
}
// Specialize for string.
template <>
void Writer_::WriteField_(const std::string &name, const std::string &value) {
    WriteFieldName_(name);
    out_ << '"' << value << "\",\n";
}

void Writer_::WriteScene(const Scene &scene) {
    if (WriteObjHeader_(scene))
        return;
    WriteObjField_("camera", scene.GetCamera(), &Writer_::WriteCamera_);
    WriteObjListField_("lights", scene.GetLights(), &Writer_::WritePointLight_);
    WriteObjListField_("render_passes", scene.GetRenderPasses(),
                       &Writer_::WriteRenderPass_);
    WriteObjFooter_();
    out_ << "\n";
}

void Writer_::WriteFieldName_(const std::string &name) {
    out_ << Indent_() << name << ": ";
}

void Writer_::WriteCamera_(const Camera &camera) {
    if (WriteObjHeader_(camera))
        return;
    Camera default_cam;
    if (camera.GetPosition() != default_cam.GetPosition())
        WriteField_("position", camera.GetPosition());
    if (camera.GetOrientation() != default_cam.GetOrientation())
        WriteField_("orientation", camera.GetOrientation());
    if (camera.GetFOV() != default_cam.GetFOV())
        WriteField_("fov", camera.GetFOV());
    if (camera.GetNear() != default_cam.GetNear())
        WriteField_("near", camera.GetNear());
    if (camera.GetFar() != default_cam.GetFar())
        WriteField_("far", camera.GetFar());
    WriteObjFooter_();
}

void Writer_::WritePointLight_(const PointLight &light) {
    if (WriteObjHeader_(light))
        return;
    PointLight default_light;
    if (light.GetPosition() != default_light.GetPosition())
        WriteField_("position", light.GetPosition());
    if (light.GetColor() != default_light.GetColor())
        WriteField_("color", light.GetColor());
    if (light.CastsShadows() != default_light.CastsShadows())
        WriteField_("cast_shadows", light.CastsShadows());
    WriteObjFooter_();
}

void Writer_::WriteRenderPass_(const RenderPass &pass) {
    if (WriteObjHeader_(pass))
        return;
    WriteObjField_("root", pass.GetRootNode(), &Writer_::WriteNode_);
    WriteObjFooter_();
}

void Writer_::WriteNode_(const Node &node) {
    // Handle derived classes.
    if (node.GetTypeName() == "TextNode") {
        WriteTextNode_(static_cast<const TextNode &>(node));
        return;
    }
    if (WriteObjHeader_(node))
        return;
    if (node.GetDisabledFlags().HasAny())
        WriteFlagsField_("disabled_flags", node.GetDisabledFlags());
    if (node.GetScale() != Vector3f(1, 1, 1))
        WriteField_("scale", node.GetScale());
    if (! node.GetRotation().IsIdentity())
        WriteField_("rotation", node.GetRotation());
    if (node.GetTranslation() != Vector3f::Zero())
        WriteField_("translation", node.GetTranslation());

    WriteObjField_("state_table", node.GetStateTable(),
                   &Writer_::WriteStateTable_);
    WriteObjField_("shader", node.GetShaderProgram(),
                   &Writer_::WriteShaderProgram_);
    WriteObjListField_("textures", node.GetTextures(), &Writer_::WriteTexture_);
    WriteObjListField_("uniforms", node.GetUniforms(), &Writer_::WriteUniform_);
    WriteObjListField_("shapes",   node.GetShapes(),   &Writer_::WriteShape_);
    WriteObjListField_("children", node.GetChildren(), &Writer_::WriteNode_);

    WriteObjFooter_();
}

void Writer_::WriteStateTable_(const StateTable &table) {
    if (WriteObjHeader_(table))
        return;
    StateTable default_table;
    if (table.GetClearColor() != default_table.GetClearColor())
        WriteField_("clear_color", table.GetClearColor());
    if (table.GetClearDepth() != default_table.GetClearDepth())
        WriteField_("clear_depth", table.GetClearDepth());
    if (table.GetLineWidth() != default_table.GetLineWidth())
        WriteField_("line_width", table.GetLineWidth());
    if (table.IsDepthTestEnabled() != default_table.IsDepthTestEnabled())
        WriteField_("depth_test_enabled", table.IsDepthTestEnabled());
    if (table.IsCullFaceEnabled() != default_table.IsCullFaceEnabled())
        WriteField_("cull_face_enabled", table.IsCullFaceEnabled());
    if (table.GetCullFaceMode() != default_table.GetCullFaceMode())
        WriteEnumField_("cull_face_mode", table.GetCullFaceMode());
    WriteObjFooter_();
}

void Writer_::WriteShaderProgram_(const ShaderProgram &program) {
    if (WriteObjHeader_(program))
        return;
    ShaderProgram default_program;
    if (program.ShouldInheritUniforms() !=
        default_program.ShouldInheritUniforms())
        WriteField_("inherit_uniforms", program.ShouldInheritUniforms());
    WriteObjListField_("uniform_defs", program.GetUniformDefs(),
                       &Writer_::WriteUniformDef_);
    WriteObjField_("vertex_source", program.GetVertexSource(),
                   &Writer_::WriteShaderSource_);
    WriteObjField_("geometry_source", program.GetGeometrySource(),
                   &Writer_::WriteShaderSource_);
    WriteObjField_("fragment_source", program.GetFragmentSource(),
                   &Writer_::WriteShaderSource_);
    WriteObjFooter_();
}

void Writer_::WriteUniformDef_(const UniformDef &def) {
    if (WriteObjHeader_(def))
        return;
    WriteEnumField_("value_type", def.GetValueType());
    WriteObjFooter_();
}

void Writer_::WriteShaderSource_(const ShaderSource &src) {
    if (WriteObjHeader_(src))
        return;
    WriteField_("path", src.GetFilePath());
    WriteObjFooter_();
}

void Writer_::WriteTexture_(const Texture &tex) {
    if (WriteObjHeader_(tex))
        return;
    if (tex.GetCount() != 1)
        WriteField_("count", tex.GetCount());
    if (! tex.GetUniformName().empty())
        WriteField_("uniform_name", tex.GetUniformName());
    WriteObjField_("image",     tex.GetImage(),   &Writer_::WriteImage_);
    WriteObjField_("sampler",   tex.GetSampler(), &Writer_::WriteSampler_);
    WriteObjFooter_();
}

void Writer_::WriteImage_(const Image &image) {
    if (WriteObjHeader_(image))
        return;
    const std::string &type = image.GetTypeName();
    if (type == "FileImage")
        WriteFileImage_(static_cast<const FileImage &>(image));
    else if (type == "ProceduralImage")
        WriteProceduralImage_(static_cast<const ProceduralImage &>(image));
    else {
        ASSERTM(false, "Unknown type for image");
    }
    WriteObjFooter_();
}

void Writer_::WriteFileImage_(const FileImage &image) {
    WriteField_("path", image.GetFilePath());
}

void Writer_::WriteProceduralImage_(const ProceduralImage &image) {
    WriteField_("function", image.GetFunctionName());
}

void Writer_::WriteSampler_(const Sampler &sampler) {
    if (WriteObjHeader_(sampler))
        return;
    Sampler default_sampler;
    if (sampler.IsAutoMipmapsEnabled() !=
        default_sampler.IsAutoMipmapsEnabled())
        WriteField_("auto_mipmaps", sampler.IsAutoMipmapsEnabled());
    if (sampler.GetCompareMode() != default_sampler.GetCompareMode())
        WriteEnumField_("compare_mode", sampler.GetCompareMode());
    if (sampler.GetCompareFunction() != default_sampler.GetCompareFunction())
        WriteEnumField_("compare_function", sampler.GetCompareFunction());
    if (sampler.GetMinFilter() != default_sampler.GetMinFilter())
        WriteEnumField_("min_filter", sampler.GetMinFilter());
    if (sampler.GetMagFilter() != default_sampler.GetMagFilter())
        WriteEnumField_("mag_filter", sampler.GetMagFilter());
    if (sampler.GetWrapRMode() != default_sampler.GetWrapRMode())
        WriteEnumField_("wrap_r_mode", sampler.GetWrapRMode());
    if (sampler.GetWrapSMode() != default_sampler.GetWrapSMode())
        WriteEnumField_("wrap_s_mode", sampler.GetWrapSMode());
    if (sampler.GetWrapTMode() != default_sampler.GetWrapTMode())
        WriteEnumField_("wrap_t_mode", sampler.GetWrapTMode());
    if (sampler.GetMaxAnisotropy() != default_sampler.GetMaxAnisotropy())
        WriteField_("max_anisotropy", sampler.GetMaxAnisotropy());
    if (sampler.GetMinLOD() != default_sampler.GetMinLOD())
        WriteField_("min_lod", sampler.GetMinLOD());
    if (sampler.GetMaxLOD() != default_sampler.GetMaxLOD())
        WriteField_("max_lod", sampler.GetMaxLOD());
    WriteObjFooter_();
}

void Writer_::WriteUniform_(const Uniform &uniform) {
    if (WriteObjHeader_(uniform))
        return;
    if (uniform.GetCount() != 1)
        WriteField_("count", uniform.GetCount());
    const std::string &lf = uniform.GetLastFieldSet();
    if (lf == "float_val")
        WriteField_(lf, uniform.GetFloat());
    else if (lf == "int_val")
        WriteField_(lf, uniform.GetInt());
    else if (lf == "uint_val")
        WriteField_(lf, uniform.GetUInt());
    else if (lf == "vec2f_val")
        WriteField_(lf, uniform.GetVector2f());
    else if (lf == "vec3f_val")
        WriteField_(lf, uniform.GetVector3f());
    else if (lf == "vec4f_val")
        WriteField_(lf, uniform.GetVector4f());
    else if (lf == "vec2i_val")
        WriteField_(lf, uniform.GetVector2i());
    else if (lf == "vec3i_val")
        WriteField_(lf, uniform.GetVector3i());
    else if (lf == "vec4i_val")
        WriteField_(lf, uniform.GetVector4i());
    else if (lf == "vec2ui_val")
        WriteField_(lf, uniform.GetVector2ui());
    else if (lf == "vec3ui_val")
        WriteField_(lf, uniform.GetVector3ui());
    else if (lf == "vec4ui_val")
        WriteField_(lf, uniform.GetVector4ui());
    else if (lf == "mat2_val")
        WriteField_(lf, uniform.GetMatrix2f());
    else if (lf == "mat3_val")
        WriteField_(lf, uniform.GetMatrix3f());
    else if (lf == "mat4_val")
        WriteField_(lf, uniform.GetMatrix4f());
    WriteObjFooter_();
}

void Writer_::WriteTextNode_(const TextNode &text) {
    if (WriteObjHeader_(text))
        return;
    TextNode default_text;
    if (text.GetText() != default_text.GetText())
        WriteField_("text", text.GetText());
    if (text.GetFontName() != default_text.GetFontName())
        WriteField_("font_name", text.GetFontName());
    if (text.GetFontSize() != default_text.GetFontSize())
        WriteField_("font_size", text.GetFontSize());
    if (text.GetSDFPadding() != default_text.GetSDFPadding())
        WriteField_("sdf_padding", text.GetSDFPadding());
    if (text.GetMaxImageSize() != default_text.GetMaxImageSize())
        WriteField_("max_image_size", text.GetMaxImageSize());
    if (text.GetColor() != default_text.GetColor())
        WriteField_("color", text.GetColor());
    if (text.GetOutlineColor() != default_text.GetOutlineColor())
        WriteField_("outline_color", text.GetOutlineColor());
    if (text.GetOutlineWidth() != default_text.GetOutlineWidth())
        WriteField_("outline_width", text.GetOutlineWidth());
    if (text.GetHalfSmoothWidth() != default_text.GetHalfSmoothWidth())
        WriteField_("half_smooth_width", text.GetHalfSmoothWidth());
    WriteObjField_("layout", text.GetLayoutOptions(),
                   &Writer_::WriteLayoutOptions_);
    WriteObjFooter_();
}

void Writer_::WriteLayoutOptions_(const LayoutOptions &opts) {
    if (WriteObjHeader_(opts))
        return;
    LayoutOptions default_opts;
    if (opts.GetTargetPoint() != default_opts.GetTargetPoint())
        WriteField_("target_point", opts.GetTargetPoint());
    if (opts.GetTargetSize() != default_opts.GetTargetSize())
        WriteField_("target_size", opts.GetTargetSize());
    if (opts.GetHAlignment() != default_opts.GetHAlignment())
        WriteEnumField_("halignment", opts.GetHAlignment());
    if (opts.GetVAlignment() != default_opts.GetVAlignment())
        WriteEnumField_("valignment", opts.GetVAlignment());
    if (opts.GetLineSpacing() != default_opts.GetLineSpacing())
        WriteField_("line_spacing", opts.GetLineSpacing());
    if (opts.GetGlyphSpacing() != default_opts.GetGlyphSpacing())
        WriteField_("glyph_spacing", opts.GetGlyphSpacing());
    if (opts.IsUsingMetricsBasedAlignment() !=
        default_opts.IsUsingMetricsBasedAlignment())
        WriteField_("use_metrics", opts.IsUsingMetricsBasedAlignment());
    WriteObjFooter_();
}

void Writer_::WriteShape_(const Shape &shape) {
    if (WriteObjHeader_(shape))
        return;
    const std::string &type = shape.GetTypeName();
    if (type == "Box")
        WriteBox_(static_cast<const Box &>(shape));
    else if (type == "Cylinder")
        WriteCylinder_(static_cast<const Cylinder &>(shape));
    else if (type == "Ellipsoid")
        WriteEllipsoid_(static_cast<const Ellipsoid &>(shape));
    else if (type == "ImportedShape")
        WriteImportedShape_(static_cast<const ImportedShape &>(shape));
    else if (type == "Line")
        WriteLine_(static_cast<const Line &>(shape));
    else if (type == "Polygon")
        WritePolygon_(static_cast<const Polygon &>(shape));
    else if (type == "Rectangle")
        WriteRectangle_(static_cast<const Rectangle &>(shape));
    else {
        ASSERTM(false, "Unknown type for shape");
    }
    WriteObjFooter_();
}

void Writer_::WriteBox_(const Box &box) {
    Box default_box;
    if (box.GetSize() != default_box.GetSize())
        WriteField_("size", box.GetSize());
}

void Writer_::WriteCylinder_(const Cylinder &cyl) {
    Cylinder default_cyl;
    if (cyl.GetBottomRadius() != default_cyl.GetBottomRadius())
        WriteField_("bottom_radius", cyl.GetBottomRadius());
    if (cyl.GetTopRadius() != default_cyl.GetTopRadius())
        WriteField_("top_radius", cyl.GetTopRadius());
    if (cyl.GetHeight() != default_cyl.GetHeight())
        WriteField_("height", cyl.GetHeight());
    if (cyl.HasTopCap() != default_cyl.HasTopCap())
        WriteField_("has_top_cap", cyl.HasTopCap());
    if (cyl.HasBottomCap() != default_cyl.HasBottomCap())
        WriteField_("has_bottom_cap", cyl.HasBottomCap());
    if (cyl.GetShaftBandCount() != default_cyl.GetShaftBandCount())
        WriteField_("shaft_band_count", cyl.GetShaftBandCount());
    if (cyl.GetCapBandCount() != default_cyl.GetCapBandCount())
        WriteField_("cap_band_count", cyl.GetCapBandCount());
    if (cyl.GetSectorCount() != default_cyl.GetSectorCount())
        WriteField_("sector_count", cyl.GetSectorCount());
}

void Writer_::WriteImportedShape_(const ImportedShape &imp) {
    ImportedShape default_imp;
    WriteField_("path", imp.GetFilePath());
    if (imp.ShouldAddNormals() != default_imp.ShouldAddNormals())
        WriteField_("add_normals", imp.ShouldAddNormals());
    if (imp.ShouldAddTexCoords() != default_imp.ShouldAddTexCoords())
        WriteField_("add_texcoords", imp.ShouldAddTexCoords());
    if (imp.GetTexDimensions() != default_imp.GetTexDimensions())
        WriteField_("tex_dimensions", imp.GetTexDimensions());
}

void Writer_::WriteEllipsoid_(const Ellipsoid &ell) {
    Ellipsoid default_ell;
    if (ell.GetLongitudeStart() != default_ell.GetLongitudeStart())
        WriteField_("longitude_start", ell.GetLongitudeStart());
    if (ell.GetLongitudeEnd() != default_ell.GetLongitudeEnd())
        WriteField_("longitude_end", ell.GetLongitudeEnd());
    if (ell.GetLatitudeStart() != default_ell.GetLatitudeStart())
        WriteField_("latitude_start", ell.GetLatitudeStart());
    if (ell.GetLatitudeEnd() != default_ell.GetLatitudeEnd())
        WriteField_("latitude_end", ell.GetLatitudeEnd());
    if (ell.GetBandCount() != default_ell.GetBandCount())
        WriteField_("band_count", ell.GetBandCount());
    if (ell.GetSectorCount() != default_ell.GetSectorCount())
        WriteField_("sector_count", ell.GetSectorCount());
    if (ell.GetSize() != default_ell.GetSize())
        WriteField_("size", ell.GetSize());
}

void Writer_::WriteLine_(const Line &line) {
    Line default_line;
    if (line.GetEnd0() != default_line.GetEnd0())
        WriteField_("end0", line.GetEnd0());
    if (line.GetEnd1() != default_line.GetEnd1())
        WriteField_("end1", line.GetEnd1());
}

void Writer_::WritePolygon_(const Polygon &poly) {
    Polygon default_poly;
    if (poly.GetSides() != default_poly.GetSides())
        WriteField_("sides", poly.GetSides());
    if (poly.GetPlaneNormal() != default_poly.GetPlaneNormal())
        WriteEnumField_("plane_normal", poly.GetPlaneNormal());
}

void Writer_::WriteRectangle_(const Rectangle &rect) {
    Rectangle default_rect;
    if (rect.GetSize() != default_rect.GetSize())
        WriteField_("size", rect.GetSize());
    if (rect.GetPlaneNormal() != default_rect.GetPlaneNormal())
        WriteEnumField_("plane_normal", rect.GetPlaneNormal());
}

bool Writer_::WriteObjHeader_(const Object &obj) {
    const bool is_instance = ! obj.GetName().empty() &&
        Util::MapContains(written_named_objects_, &obj);

    out_ << obj.GetTypeName();
    if (! obj.GetName().empty())
        out_ << " \"" << obj.GetName() << "\"";
    if (is_instance) {
        out_ << ";";
        return true;
    }
    else {
        if (! obj.GetName().empty())
            written_named_objects_.insert(&obj);
        out_ << " {\n";
        ++cur_depth_;
        return false;
    }
}

void Writer_::WriteObjFooter_() {
    --cur_depth_;
    out_ << Indent_() << "}";
}

// ----------------------------------------------------------------------------
// Writer implementation.
// ----------------------------------------------------------------------------

Writer::Writer() {
}

Writer::~Writer() {
}

void Writer::WriteScene(const Scene &scene, std::ostream &out) {
    Writer_ writer(out);
    writer.WriteScene(scene);
}

}  // namespace SG
