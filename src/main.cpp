#include <GLFW/glfw3.h>

#include <signal.h> // XXXX

#include <iostream>
#include <memory>

#include <ion/math/range.h>
#include <ion/math/vector.h>

#include "Application.h"
#include "Interfaces/IRenderer.h"
#include "Interfaces/IScene.h"
#include "Interfaces/IVR.h"

// This is used to toggle between paused mode (for debugging) and regular
// running mode.
static bool paused = false;

static void
GLFWErrorCallback(int error, const char *description) {
    std::cerr << "*** Error " << error << ": " << description << "\n";
}

static void GLFWKeyCallback(GLFWwindow *window, int key,
                            int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        else if (key == GLFW_KEY_P) {
            paused = ! paused;
            std::cout << "=== Now in " << (paused ? "paused" : "regular")
                      << " mode\n";
        }
    }
}

static ion::math::Range2i GetWindowViewport(GLFWwindow &window) {
    int width, height;
    glfwGetWindowSize(&window, &width, &height);

    return ion::math::Range2i::BuildWithSize(
        ion::math::Point2i(0, 0), ion::math::Vector2i(width, height));
}

int main() {
    std::unique_ptr<Application> app(new Application);

    try {
        bool using_vr = app->GetVR().Init();

        if (! glfwInit()) {
            std::cerr << "*** GLFW initialization failed!\n";
            return 1;
        }

        glfwSetErrorCallback(GLFWErrorCallback);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        GLFWwindow *window = glfwCreateWindow(800, 600, "GLFW Test",
                                              nullptr, nullptr);
        if (! window) {
            std::cerr << "*** GLFW window creation failed!\n";
        }

        glfwMakeContextCurrent(window);

        glfwSetKeyCallback(window, GLFWKeyCallback);

        if (using_vr)
            app->GetVR().InitRendering(app->GetRenderer());

        while (! glfwWindowShouldClose(window)) {
            if (using_vr) {
                if (! app->GetVR().PollEvents())
                    break;
                // Render to VR headset.
                app->GetVR().Render(app->GetScene(), app->GetRenderer());
            }

            glfwMakeContextCurrent(window); // Need to set context again.
            app->GetRenderer().RenderScene(app->GetScene(),
                                           GetWindowViewport(*window));

            glfwSwapBuffers(window);

            if (paused)
                glfwWaitEvents();
            else
                glfwPollEvents();
        }

        app.reset(nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }
    catch (IVR::VRException &ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }

    std::cerr << "XXXX Killing app\n";
    raise(SIGTERM); // XXXX Force exit, since SteamVR hangs!

    return 0;
}
