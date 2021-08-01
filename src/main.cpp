#include <GLFW/glfw3.h>

#include <signal.h> // XXXX

#include <iostream>
#include <memory>

#include "gfx.h"
#include "vr.h"

static void
GLFWErrorCallback(int error, const char *description) {
    std::cerr << "*** Error " << error << ": " << description << "\n";
}

static void GLFWKeyCallback(GLFWwindow *window, int key,
                            int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main() {
    std::unique_ptr<VR> vr(new VR());

    try {
        bool using_vr = vr->Init();

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

        std::shared_ptr<GFX> gfx(new GFX());

        if (using_vr)
            vr->InitDraw(gfx);

        while (! glfwWindowShouldClose(window)) {
            if (using_vr) {
                if (! vr->PollEvents())
                    break;
                vr->Draw();   // Draw to headset.
            }

            glfwMakeContextCurrent(window); // Needed to set context again.
            gfx->Draw(800, 600);  // Draw to window.

            glfwSwapBuffers(window);
            glfwWaitEvents();
        }

        gfx = nullptr;

        glfwDestroyWindow(window);
        glfwTerminate();
    }
    catch (VR::VRException &ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }

    std::cerr << "XXXX Killing app\n";
    raise(SIGTERM); // XXXX Force exit, since SteamVR hangs!

    return 0;
}
