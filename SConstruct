from ionsetup import IonSetup

optimize = False

env = Environment(
    CPPPATH = [
        '/local/github/tmp/glfw/include',
    ],
    LIBPATH = '/local/github/tmp/glfw/src',
    LIBS    = ['glfw3', 'pthread', 'dl'],
    CXXFLAGS = ['-g'],
    LINKFLAGS = ['-g'],
)

# Add Ion settings.
IonSetup(env, mode = 'opt' if optimize else 'dbg', root_dir = '/local/inst/ion')

env.Program('glfwtest', ['glfwtest.cpp', 'gfx.cpp'])
