#pragma once

#include <vector>

#include <ion/gfx/renderer.h>
#include <ion/gfx/uniformblock.h>
#include <ion/gfxutils/shadermanager.h>

#include "Assert.h"
#include "Renderer.h"
#include "SG/RenderData.h"
#include "SG/ShaderProgram.h"
#include "SG/Typedefs.h"

struct FBTarget;

namespace SG {

/// RenderPass is a base class for an object representing one pass during
/// multipass rendering.
class RenderPass : public Object {
  public:
    virtual void AddFields() override;

    const std::vector<ShaderProgramPtr> & GetShaderPrograms() const {
        return shader_programs_;
    }

    /// Sets up an Ion ShaderProgram for each ShaderProgram in the RenderPass.
    void SetUpIon(Tracker &tracker,
                  ion::gfxutils::ShaderManager &shader_manager);

    /// Convenience that returns the default ShaderProgram for the RenderPass.
    /// This is defined as the first ShaderProgram added to the RenderPass.
    ShaderProgramPtr GetDefaultShaderProgram() const;

    /// Renders the pass using the given RenderData and Ion renderer. If
    /// fb_target is not null, it should be used as the render target.
    virtual void Render(ion::gfx::Renderer &renderer, RenderData &data,
                        const FBTarget *fb_target = nullptr) = 0;

  protected:
    /// Convenience that returns the named ShaderProgram from the
    /// RenderPass. Returns a null pointer if there is no such program.
    ShaderProgramPtr FindShaderProgram(const std::string &name) const;

    /// Sets a uniform by name in an Ion UniformBlock, asserting if it
    /// fails. Returns true if successful.
    template <typename T>
    bool SetIonUniform(ion::gfx::UniformBlock &block, const std::string &name,
                       const T &value) {
        bool ok = block.SetUniformByName(name, value);
        ASSERTM(ok, "Setting uniform " + name + " in " + GetDesc());
        return ok;
    }

    /// Sets one value in an array uniform by name in an Ion UniformBlock,
    /// asserting if it fails. Returns true if successful.
    template <typename T>
    bool SetIonUniformAt(ion::gfx::UniformBlock &block, const std::string &name,
                         size_t index, const T &value) {
        bool ok = block.SetUniformByNameAt(name, index, value);
        ASSERTM(ok, "Setting array uniform " + name + " in " + GetDesc());
        return ok;
    }

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectListField<ShaderProgram> shader_programs_{"shader_programs"};
    ///@}
};

}  // namespace SG
