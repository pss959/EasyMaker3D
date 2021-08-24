#pragma once

#include "Parser/ObjectSpec.h"
#include "SG/Object.h"
#include "SG/Typedefs.h"

namespace SG {

//! A RenderPass object represents one pass during multipass rendering. It has
//! no Ion equivalent.
class RenderPass : public Object {
  public:
    //! Types of render passes.
    enum class Type {
        kShadow,    //!< Pass used to compute depth maps for shadows.
        kLighting,  //!< Pass used to compute final lighting.
    };

    Type                     GetType()     const { return type_; }
    const ShaderProgramPtr & GetShader()   const { return shader_; }
    const NodePtr &          GetRootNode() const { return root_; }

    virtual void SetUpIon(IonContext &context) override;

    static Parser::ObjectSpec GetObjectSpec();

  private:
    //! \name Parsed Fields
    //!@{
    Type             type_ = Type::kLighting;  //!< Type of render pass.
    ShaderProgramPtr shader_;                  //!< Shader used to render.
    NodePtr          root_;                    //!< Root node to render.
    //!@}
};

}  // namespace SG
