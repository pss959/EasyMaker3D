#pragma once

#include <vector>

#include <ion/gfx/renderer.h>

#include "Assert.h"
#include "Renderer.h"
#include "Math/Types.h"
#include "SG/ShaderNode.h"
#include "SG/ShaderProgram.h"
#include "SG/Typedefs.h"

struct FBTarget;

namespace SG {

/// RenderPass is a base class for an object representing one pass during
/// multipass rendering.
class RenderPass : public Object {
  public:
    /// This struct encapsulates all of the data used by derived classes to
    /// render.
    // XXXX Move this somewhere else???
    struct PassData {
        /// Per-light data for rendering.
        struct LightData {
            Point3f              position;       ///< From light (world coords).
            Color                color;          ///< From light.
            bool                 casts_shadows;  ///< From light.
            Matrix4f             light_matrix;   ///< Computed by ShadowPass.
            ion::gfx::TexturePtr shadow_map;     ///< Generated by ShadowPass.
        };

        Range2i                viewport;         ///< From View.
        Matrix4f               proj_matrix;      ///< Computed from View.
        Matrix4f               view_matrix;      ///< Computed from View.
        Point3f                view_pos;         ///< Set from View.
        std::vector<LightData> per_light;        ///< LightData per light.

        /// Framebuffer target for rendering. XXXX Get rid of this?
        const FBTarget         *fb_target = nullptr;
    };

    virtual void AddFields() override;

    const std::vector<ShaderProgramPtr> & GetShaderPrograms() const {
        return shader_programs_;
    }
    const ShaderNodePtr & GetRootNode() const { return root_node_; }

    /// Convenience that returns the named ShaderProgram from the
    /// RenderPass. Returns a null pointer if there is no such program.
    ShaderProgramPtr FindShaderProgram(const std::string &name) const;

    /// Convenience that returns the default ShaderProgram for the RenderPass.
    /// This is found by looking at the shader name in the root ShaderNode and
    /// then finding the named shader in the RenderPass.
    ShaderProgramPtr GetDefaultShaderProgram() const;

    /// Sets values in the RenderPass's uniform block.
    virtual void SetUniforms(PassData &data) = 0;

    /// Renders the pass using the given PassData and Ion renderer.
    virtual void Render(ion::gfx::Renderer &renderer, PassData &data) = 0;

  protected:
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
    Parser::ObjectField<ShaderNode>        root_node_{"root_node"};
    ///@}
};

}  // namespace SG
