#pragma once

#include <ion/gfx/image.h>

#include "NParser/FieldSpec.h"
#include "SG/Resource.h"

namespace SG {

//! A ShaderSource object encapsulates a GLSL shader read from a file.  It is
//! derived from Resource so that it reads and maintains the path to the file
//! the source was read from.
class ShaderSource : public Resource {
  public:
    //! Returns a string containing the shader source code.
    const std::string & GetSourceString() const { return source_string_; }

  private:
    std::string source_string_;

    //! Redefines this to read the source.
    virtual void Finalize() override;
};

}  // namespace SG
