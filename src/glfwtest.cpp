#include <GLFW/glfw3.h>

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

    if (! vr->Init()) {
        std::cerr << "*** VR initialization failed!\n";
        return 1;
    }
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

    {
        GFX gfx(width, height);

        while (! glfwWindowShouldClose(window)) {
            gfx.Draw();
            glfwSwapBuffers(window);
            glfwWaitEvents();
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
