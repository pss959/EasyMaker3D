#pragma once

#include <vector>

#include <ion/gfx/framebufferobject.h>
#include <ion/gfx/texture.h>

#include "SG/RenderPass.h"
#include "SG/Typedefs.h"

namespace SG {

//! ShadowPass is a derived RenderPass that represents a shadow pass during
//! multipass rendering. It actually does one render pass per light.
class ShadowPass : public RenderPass {
  public:
    virtual void AddFields() override;
    virtual void SetUpIon(const ContextPtr &context) override;
    virtual void Render(ion::gfx::Renderer &renderer, PassData &data) override;
    virtual PassType GetPassType() const override {
        return PassType::kShadowPass;
    }

  private:
    //! Struct storing items needed for each light.
    struct PerLight_ {
        //! Texture that holds the shadow depth map.
        ion::gfx::TexturePtr           texture;
        //! FramebufferObject that holds the result of rendering the depth.
        ion::gfx::FramebufferObjectPtr fbo;
    };

    //! Per-light Ion infrastructure for creating shadow maps.
    std::vector<PerLight_> per_light_;

    //! Creates the PerLight_ data for the indexed light in the PassData.
    void CreatePerLightData_(PassData &data, size_t index);

    //! Updates some fields in the PassData::LightData instance.
    void SetPerLightData_(const PerLight_ &pldata, PassData::LightData &data);
};

}  // namespace SG
