#pragma once

#include <string>

#include "Parser/ObjectSpec.h"
#include "SG/Object.h"
#include "Util/FilePath.h"

namespace SG {

//! A ShaderSource object encapsulates a GLSL shader read from a file.
class ShaderSource : public Object {
  public:
    //! Returns the path that the shader was read from.
    Util::FilePath GetFilePath() const { return path_; }

    //! Returns a string containing the shader source code.
    const std::string & GetSourceString() const { return source_string_; }

    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    //! \name Parsed Fields
    //!@{
    std::string path_;
    //!@}

    std::string source_string_;
};

}  // namespace SG
