#pragma once

#include <string>

#include "SG/Object.h"
#include "Util/FilePath.h"

namespace Parser { class Registry; }

namespace SG {

/// A ShaderSource object encapsulates a GLSL shader read from a file.
class ShaderSource : public Object {
  public:
    virtual void AddFields() override;

    /// Returns the path that the shader was read from.
    Util::FilePath GetFilePath() const { return path_.GetValue(); }

  protected:
    ShaderSource() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> path_{"path"};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
