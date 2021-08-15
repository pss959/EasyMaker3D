#pragma once

#include "Graph/Resource.h"

namespace Input { class Extractor; }

namespace Graph {

//! A ShaderSource object encapsulates a GLSL shader read from a file.
class ShaderSource : public Resource {
  public:
    //! Returns a string containing the shader source code.
    const std::string & GetSourceString() const { return source_string_; }

  private:
    std::string source_string_;  //! Shader source code as a string.

    // XXXX
    void SetSourceString_(const std::string &s) { source_string_ = s; }

    friend class ::Input::Extractor;
};

}  // namespace Graph
