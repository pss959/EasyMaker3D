#pragma once

#include "Graph/Resource.h"

namespace Graph {

//! A ShaderSource object encapsulates a GLSL shader read from a file.
class ShaderSource : public Resource {
  public:
    //! Returns a string containing the shader source code.
    const std::string & GetSource() const { return source_; }

  private:
    std::string source_;  //! Shader source code as a string.

    friend class Reader_;
};

}  // namespace Graph
