#include "SG/Reader.h"

#include "NParser/Parser.h"
#include "SG/Box.h"
#include "SG/Camera.h"
#include "SG/Cylinder.h"
#include "SG/Ellipsoid.h"
#include "SG/Image.h"
#include "SG/Node.h"
#include "SG/Polygon.h"
#include "SG/Writer.h"
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

// ----------------------------------------------------------------------------
// This internal class does most of the work.
// ----------------------------------------------------------------------------

class Writer_ {
  public:
    //! The constructor is passed the output stream.
    Writer_(std::ostream &out) : out_(out) {}

    void WriteScene(const Scene &scene);

  private:
    std::ostream    &out_;            //!< Stream passed to constructor.
    int              cur_depth_ = 0;  //!< Current depth in graph.
    static const int kIndent_   = 2;  //!< Spaces to indent each level.

    void WriteCamera_(const Camera &camera);
    void WriteNode_(const Node &node);
    void WriteStateTable_(const StateTable &table);

    void WriteFieldName_(const std::string &name);
    void WriteObjHeader_(const Object &obj);
    void WriteObjFooter_();

    template <typename T>
    void WriteField_(const std::string &name, const T &value);

    std::string Indent_() { return std::string(kIndent_ * cur_depth_, ' '); }
};

template <typename T>
void Writer_::WriteField_(const std::string &name, const T &value) {
    WriteFieldName_(name);
    out_ << value << ",\n";
}

// Specialize for bool
template <> void
Writer_::WriteField_(const std::string &name, const bool &value) {
    WriteFieldName_(name);
    out_ << std::boolalpha << value << ",\n";
}

void Writer_::WriteScene(const Scene &scene) {
    WriteObjHeader_(scene);
    if (scene.GetCamera()) {
        WriteFieldName_("camera");
        WriteCamera_(*scene.GetCamera());
    }
    if (scene.GetRootNode()) {
        WriteFieldName_("root");
        WriteNode_(*scene.GetRootNode());
    }
    WriteObjFooter_();
}

void Writer_::WriteFieldName_(const std::string &name) {
    out_ << Indent_() << name << ": ";
}

void Writer_::WriteCamera_(const Camera &camera) {
    WriteObjHeader_(camera);
    Camera default_cam;
    if (camera.GetPosition() != default_cam.GetPosition())
        WriteField_("position", camera.GetPosition());
    if (camera.GetOrientation() != default_cam.GetOrientation())
        WriteField_("orientation", camera.GetOrientation());
    const Camera::FOV &fov = camera.GetFOV();
    if (fov.left  != default_cam.GetFOV().left ||
        fov.right != default_cam.GetFOV().right ||
        fov.up    != default_cam.GetFOV().up ||
        fov.down  != default_cam.GetFOV().down)
        WriteField_("fov", Vector4f(fov.left.Degrees(),
                                    fov.right.Degrees(),
                                    fov.up.Degrees(),
                                    fov.down.Degrees()));
    if (camera.GetNear() != default_cam.GetNear())
        WriteField_("near", camera.GetNear());
    if (camera.GetFar() != default_cam.GetFar())
        WriteField_("far", camera.GetFar());
    WriteObjFooter_();
}

void Writer_::WriteNode_(const Node &node) {
    WriteObjHeader_(node);
    if (node.GetScale() != Vector3f(1, 1, 1))
        WriteField_("scale", node.GetScale());
    if (! node.GetRotation().IsIdentity())
        WriteField_("rotation", node.GetRotation());
    if (node.GetTranslation() != Vector3f::Zero())
        WriteField_("translation", node.GetTranslation());

    if (node.GetStateTable()) {
        WriteFieldName_("state_table");
        WriteStateTable_(*node.GetStateTable());
    }

    // XXXX Contents.
    WriteObjFooter_();
}

void Writer_::WriteStateTable_(const StateTable &table) {
    StateTable default_table;
    WriteObjHeader_(table);
    if (table.GetClearColor() != default_table.GetClearColor())
        WriteField_("clear_color", table.GetClearColor());
    if (table.IsDepthTestEnabled() != default_table.IsDepthTestEnabled())
        WriteField_("depth_test_enabled", table.IsDepthTestEnabled());
    if (table.IsCullFaceEnabled() != default_table.IsCullFaceEnabled())
        WriteField_("cull_face_enabled", table.IsCullFaceEnabled());
    WriteObjFooter_();
}

void Writer_::WriteObjHeader_(const Object &obj) {
    out_ << obj.GetTypeName();
    if (! obj.GetName().empty())
        out_ << " \"" << obj.GetName() << "\"";
    out_ << " {\n";
    ++cur_depth_;
}

void Writer_::WriteObjFooter_() {
    --cur_depth_;
    out_ << Indent_() << "}\n";
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
