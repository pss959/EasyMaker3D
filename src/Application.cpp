#include "Application.h"

#include "OpenXRVR.h"
#include "Renderer.h"
#include "Scene.h"

template <typename InterfaceType, typename RealType>
InterfaceType & GetInstance(std::unique_ptr<InterfaceType> &ptr) {
    if (! ptr)
        ptr.reset(new RealType());
    return *ptr;
}

IRenderer & Application::GetRenderer() {
    return GetInstance<IRenderer, Renderer>(renderer_);
}

IScene & Application::GetScene() {
    return GetInstance<IScene, Scene>(scene_);
}

IVR & Application::GetVR() {
    return GetInstance<IVR, OpenXRVR>(vr_);
}
