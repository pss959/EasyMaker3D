#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "SG/Image.h"

namespace SG {

//! ProceduralImage is a derived Image object that generates the Image data
//! using a named function.
class ProceduralImage : public Image {
  public:
    //! Typedef for function used to generate an Ion Image.
    typedef std::function<ion::gfx::ImagePtr()> ImageFunc;

    virtual void AddFields() override;

    //! Adds a procedural function associated with the given name. Only
    //! functions added with this can be used to generate images.
    static void AddFunction(const std::string &name, ImageFunc func) {
        func_map_[name] = func;
    }

    //! Returns the name of the function used to generate the image.
    const std::string & GetFunctionName() const { return function_; }

    virtual void SetUpIon(const ContextPtr &context) override;

  private:
    //! \name Parsed Fields
    //!@{
    Parser::TField<std::string> function_{"function"};
    //!@}

    //! Stores all registered functions by name.
    static std::unordered_map<std::string, ImageFunc> func_map_;
};

}  // namespace SG
