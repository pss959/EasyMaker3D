#include <GLFW/glfw3.h>

#include <iostream>

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
    if (! glfwInit()) {
        std::cerr << "*** GLFW initialization failed!\n";
        return 1;
    }

    glfwSetErrorCallback(GLFWErrorCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow *window = glfwCreateWindow(640, 480, "Some Title", NULL, NULL);
    if (! window) {
        std::cerr << "*** GLFW window creation failed!\n";
    }

    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, GLFWKeyCallback);

    gfx::Init(640, 480);

    if (vr::Init()) {
        while (! glfwWindowShouldClose(window)) {
            gfx::Draw();
            glfwSwapBuffers(window);
            glfwWaitEvents();
        }
    }

    gfx::CleanUp();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
