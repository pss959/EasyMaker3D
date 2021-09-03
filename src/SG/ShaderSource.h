#pragma once

#include <string>

#include "SG/Object.h"
#include "Util/FilePath.h"

namespace SG {

//! A ShaderSource object encapsulates a GLSL shader read from a file.
class ShaderSource : public Object {
  public:
    virtual void AddFields() override;

    //! Returns the path that the shader was read from.
    Util::FilePath GetFilePath() const { return path_.GetValue(); }

    //! Returns a string containing the shader source code.
    const std::string & GetSourceString() const { return source_string_; }

    virtual void SetUpIon(IonContext &context) override;

  private:
    //! \name Parsed Fields
    //!@{
    Parser::TField<std::string> path_{"path"};
    //!@}

    std::string source_string_;
};

}  // namespace SG
