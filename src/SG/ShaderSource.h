#pragma once

#include <ion/gfx/image.h>

#include "Parser/ObjectSpec.h"
#include "SG/Resource.h"

namespace SG {

//! A ShaderSource object encapsulates a GLSL shader read from a file.  It is
//! derived from Resource so that it reads and maintains the path to the file
//! the source was read from.
class ShaderSource : public Resource {
  public:
    //! Returns a string containing the shader source code.
    const std::string & GetSourceString() const { return source_string_; }

    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    std::string source_string_;
};

}  // namespace SG
