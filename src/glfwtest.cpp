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
        vr->Init();

        if (! glfwInit()) {
            std::cerr << "*** GLFW initialization failed!\n";
            return 1;
        }

        int width  = vr->GetWidth();
        int height = vr->GetHeight();

        glfwSetErrorCallback(GLFWErrorCallback);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        GLFWwindow *window = glfwCreateWindow(width, height, "GLFW Test",
                                              nullptr, nullptr);
        if (! window) {
            std::cerr << "*** GLFW window creation failed!\n";
        }

        glfwMakeContextCurrent(window);

        glfwSetKeyCallback(window, GLFWKeyCallback);

        // Use half the VR resolution.
        std::unique_ptr<GFX> gfx(new GFX(width / 2, height / 2));

        vr->CreateSession(*gfx);

        while (! glfwWindowShouldClose(window)) {
            glfwMakeContextCurrent(window); // Needed to set context again.

            if (! vr->PollEvents()) {
                std::cerr << "XXXX Exiting because of VR event\n";
                break;
            }

            vr->Draw(*gfx);
            glfwSwapBuffers(window);
            glfwWaitEvents();
        }
        std::cerr << "XXXX Exited main loop\n";

        glfwMakeContextCurrent(window); // Needed to set context again.
        gfx.reset(nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }
    catch (VR::VRException &ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }

    raise(SIGTERM); // XXXX Force exit, since SteamVR hangs!

    return 0;
}
