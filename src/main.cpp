#include <GLFW/glfw3.h>

#include <signal.h> // XXXX

#include <iostream>
#include <memory>

#include <ion/math/vector.h>

#include "Application.h"
#include "Interfaces/IViewer.h"

using ion::math::Vector2i;

static bool MainLoop(const Vector2i &default_window_size) {
    Application app;
    IApplication::Context &context = app.Init(default_window_size);
    if (context.viewers.empty())
        return false;

    while (true) {
        // Render to all viewers.
        for (auto &viewer: context.viewers)
            viewer->Render(*context.scene, *context.renderer);

        glfwWaitEvents(); // XXXX
    }

    if (context.vr) {
        std::cerr << "XXXX Killing app\n";
        raise(SIGTERM); // XXXX Force exit, since SteamVR hangs!
    }
    return true;
}

int main() {
    const Vector2i default_size(800, 600);
    return MainLoop(default_size) ? 0 : -1;
}
