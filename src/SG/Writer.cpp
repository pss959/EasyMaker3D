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

    void WriteObjHeader_(const Object &obj);
    void WriteObjFooter_(const Object &obj);

    std::string Indent_() { return std::string(kIndent_ * cur_depth_, ' '); }
};

void Writer_::WriteScene(const Scene &scene) {
    WriteObjHeader_(scene);
    // XXXX Contents!
    WriteObjFooter_(scene);
}

void Writer_::WriteObjHeader_(const Object &obj) {
    out_ << Indent_() << "XXXX " << "{\n";
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
