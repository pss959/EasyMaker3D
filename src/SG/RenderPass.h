#pragma once

#include <vector>

#include <ion/gfx/renderer.h>
#include <ion/gfx/uniformblock.h>
#include <ion/gfxutils/shadermanager.h>

#include "SG/ShaderProgram.h"
#include "Util/Assert.h"
#include "Util/Memory.h"

struct FBTarget;

namespace SG {

DECL_SHARED_PTR(IonContext);
DECL_SHARED_PTR(Node);
DECL_SHARED_PTR(RenderPass);
struct RenderData;

/// RenderPass is a base class for an object representing one pass during
/// multipass rendering.
///
/// \ingroup SG
class RenderPass : public Object {
  public:
    const std::vector<ShaderProgramPtr> & GetShaderPrograms() const {
        return shader_programs_;
    }

    /// Sets up an Ion ShaderProgram for each ShaderProgram in the RenderPass.
    void SetUpIon(const IonContextPtr &ion_context);

    /// Convenience that returns the default ShaderProgram for the RenderPass.
    /// This is defined as the first ShaderProgram added to the RenderPass.
    ShaderProgramPtr GetDefaultShaderProgram() const;

    /// Renders the pass using the given RenderData and Ion renderer. If
    /// fb_target is not null, it should be used as the render target.
    virtual void Render(ion::gfx::Renderer &renderer, RenderData &data,
                        const FBTarget *fb_target = nullptr) = 0;

  protected:
    virtual void AddFields() override;

    /// Returns all Nodes under the given root Node that contain a shader name
    /// starting with the given prefix string. This is used by derived classes
    /// to find the nodes to set uniforms in.
    static std::vector<NodePtr> FindNodesMatchingShaderName(
        const NodePtr &root, const Str &prefix);

    /// Sets a uniform by name in an Ion UniformBlock, asserting if it
    /// fails. Returns true if successful.
    template <typename T>
    bool SetIonUniform(ion::gfx::UniformBlock &block, const Str &name,
                       const T &value) {
        bool ok = block.SetUniformByName(name, value);
        ASSERTM(ok, "Setting uniform " + name + " in " + GetDesc());
        return ok;
    }

    /// Sets one value in an array uniform by name in an Ion UniformBlock,
    /// asserting if it fails. Returns true if successful.
    template <typename T>
    bool SetIonUniformAt(ion::gfx::UniformBlock &block, const Str &name,
                         size_t index, const T &value) {
        bool ok = block.SetUniformByNameAt(name, index, value);
        // Handle the case where the uniform has a single value and the index
        // is 0. Ion will not recognize this case as an array uniform.
        if (! ok && index == 0)
            ok = block.SetUniformByName(name, value);
        ASSERTM(ok, "Setting array uniform " + name + " in " + GetDesc() +
                " @ index " + Util::ToString(index));
        return ok;
    }

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectListField<ShaderProgram> shader_programs_;
    ///@}
};

}  // namespace SG
