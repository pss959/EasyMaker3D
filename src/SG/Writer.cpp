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

    void WriteFieldName_(const std::string &name);
    void WriteObjHeader_(const Object &obj);
    void WriteObjFooter_(const Object &obj);

    template <typename T>
    void WriteField_(const std::string &name, const T &value) {
        WriteFieldName_(name);
        out_ << value << ",\n";
    }
    std::string Indent_() { return std::string(kIndent_ * cur_depth_, ' '); }
};

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
    WriteObjFooter_(scene);
}

void Writer_::WriteFieldName_(const std::string &name) {
    out_ << Indent_() << name << ": ";
}

void Writer_::WriteCamera_(const Camera &camera) {
    WriteObjHeader_(camera);
    // XXXX Contents.
    WriteObjFooter_(camera);
}
void Writer_::WriteNode_(const Node &node) {
    WriteObjHeader_(node);
    if (node.GetScale() != Vector3f(1, 1, 1))
        WriteField_("scale", node.GetScale());
    if (! node.GetRotation().IsIdentity())
        WriteField_("rotation", node.GetRotation());
    if (node.GetTranslation() != Vector3f::Zero())
        WriteField_("translation", node.GetTranslation());

    // XXXX Contents.
    WriteObjFooter_(node);
}

void Writer_::WriteObjHeader_(const Object &obj) {
    out_ << obj.GetTypeName();
    if (! obj.GetName().empty())
        out_ << " \"" << obj.GetName() << "\"";
    out_ << " {\n";
    ++cur_depth_;
}

void Writer_::WriteObjFooter_(const Object &obj) {
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
