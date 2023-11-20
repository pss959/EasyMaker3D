#pragma once

#include "SG/Object.h"
#include "Util/FilePath.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(ShaderSource);

/// A ShaderSource object encapsulates a GLSL shader read from a file.
///
/// \ingroup SG
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
    Parser::TField<Str> path_;
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
