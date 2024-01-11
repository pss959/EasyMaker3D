#pragma once

#include <ion/gfx/image.h>
#include <ion/gfxutils/shadermanager.h>

#include "Math/Types.h"
#include "Util/Memory.h"

struct Frustum;
namespace SG { class Scene; }

DECL_SHARED_PTR(FBTarget);
DECL_SHARED_PTR(IRenderer);

/// IRenderer is an interface for renderer classes used by Viewer classes to
/// render a scene. It exists to allow tests to use a fake version to avoid
/// having to set up actual graphics.
///
/// \ingroup Viewers
class IRenderer {
  public:
    /// Resets everything (e.g., after a reload). The new Scene is supplied for
    /// setting up remote debugging.
    virtual void Reset(const SG::Scene &scene) = 0;

    /// \name Frame management.
    ///@{

    /// Begins a new frame. This and EndFrame() should bracket all
    /// RenderScene() calls representing the current frame.
    virtual void BeginFrame() = 0;

    /// Ends the current frame. This and BeginFrame() should bracket all
    /// RenderScene() calls representing the current frame.
    virtual void EndFrame() = 0;

    /// Returns the current frame count. The count is reset to 0 when Reset()
    /// is called.
    virtual uint64 GetFrameCount() const = 0;

    ///@}

    /// Sets a framebuffer target for subsequent calls to RenderScene() and as
    /// a source for ReadImage(). This is null by default, meaning that the
    /// default framebuffer is the target and source.
    virtual void SetFBTarget(const FBTargetPtr &fb_target) = 0;

    /// Renders the given Scene using the given Frustum.
    virtual void RenderScene(const SG::Scene &scene,
                             const Frustum &frustum) = 0;

    /// Reads a rectangle of pixels from the framebuffer and returns them in an
    /// Ion Image in kRGB888 format.
    virtual ion::gfx::ImagePtr ReadImage(const Viewport &rect) = 0;

    /// Returns the OpenGL ID of the resolved framebuffer texture from the
    /// current FBTarget. Returns 0 if there is no current FBTarget or on
    /// error.
    virtual uint32 GetResolvedTextureID() = 0;
};
