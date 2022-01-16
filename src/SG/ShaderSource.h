#pragma once

#include <string>

#include "SG/Object.h"
#include "Util/FilePath.h"

namespace Parser { class Registry; }

namespace SG {

/// A ShaderSource object encapsulates a GLSL shader read from a file.
class ShaderSource : public Object {
  public:
    /// Returns the path that the shader was read from.
    FilePath GetFilePath() const { return path_.GetValue(); }

  protected:
    ShaderSource() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> path_{"path"};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
