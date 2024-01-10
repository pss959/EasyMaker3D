#pragma once

#include <ion/gfx/image.h>
#include <ion/gfxutils/shadermanager.h>

#include "Math/Types.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(IRenderer);

struct FBTarget;
namespace SG { class Scene; }

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

    /// Renders the given Scene using the given Frustum. If fb_target is not
    /// null, it is used instead of the default target.
    virtual void RenderScene(const SG::Scene &scene, const Frustum &frustum,
                             const FBTarget *fb_target = nullptr) = 0;

    /// Given an FBTarget, this returns the OpenGL ID of the resolved
    /// framebuffer texture. Returns 0 on error.
    virtual uint32 GetResolvedTextureID(const FBTarget &fb_target) = 0;

    /// Reads a rectangle of pixels from the GLFWViewer window and returns them
    /// in an Ion Image in kRGB888 format.
    virtual ion::gfx::ImagePtr ReadImage(const Viewport &rect) = 0;
};
