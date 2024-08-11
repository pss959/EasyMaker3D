//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Viewers/IRenderer.h"

DECL_SHARED_PTR(Renderer);

/// Renderer is a derived IRenderer that is used by Viewer classes to actually
/// render a scene.
///
/// \ingroup Viewers
class Renderer : public IRenderer {
  public:
    /// The constructor passed an Ion ShaderManager and a flag indicating
    /// whether to set up Ion remote debugging.
    Renderer(const ion::gfxutils::ShaderManagerPtr &shader_manager,
             bool use_ion_remote);

    ~Renderer();

    virtual void Reset(const SG::Scene &scene) override;
    virtual void BeginFrame() override;
    virtual void EndFrame() override;
    virtual uint64 GetFrameCount() const override;
    virtual void SetFBTarget(const FBTargetPtr &fb_target) override;
    virtual void RenderScene(const SG::Scene &scene,
                             const Frustum &frustum) override;
    virtual ion::gfx::ImagePtr ReadImage(const Viewport &rect) override;
    virtual uint32 GetResolvedTextureID() override;

  private:
    class Impl_; // This class does most of the work.

    std::unique_ptr<Impl_> impl_;  ///< Implementation instance.
};
