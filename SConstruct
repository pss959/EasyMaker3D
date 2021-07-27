# -----------------------------------------------------------------------------
# Configuration.
# -----------------------------------------------------------------------------

# Set this to True or False to build debug or optimized.
optimize = False

# Set this to True or False for brief output.
brief = False

# All build products go into this directory.
build_dir = 'build'

# -----------------------------------------------------------------------------
# Environment setup.
# -----------------------------------------------------------------------------

# Load helper functions.
from brief     import Brief
from ionsetup  import IonSetup
from stringify import Stringify

# Send all build products to build_dir.
VariantDir(build_dir, 'src', duplicate = 0)

common_flags = [
    '--std=c++14',
    '-Wall',
]

env = Environment(
    CPPPATH   = [
        '#submodules/glfw/include',
    ],
    LIBPATH   = [
        f'#submodules/glfw/{build_dir}',
    ],
    CXXFLAGS  = common_flags,
    LINKFLAGS = common_flags,
    LIBS      = ['glfw3', 'pthread', 'dl'],
)

# Shorten compile/link lines for clarity
if brief:
    Brief(env)

# Create SCons's database file in the build directory for easy cleanup.
env.SConsignFile(f'{build_dir}/sconsign.dblite')

# Specialize for debug or optimized modes.
if optimize:
    env.Append(
        CXXFLAGS  = ['-O3'],
        LINKFLAGS = ['-O3', '-Wl,--strip-all'],
        CPPDEFINES = [('CHECK_GL_ERRORS', 'false')],
    )
else:
    env.Append(
        CXXFLAGS  = ['-g'],
        LINKFLAGS = ['-g'],
        CPPDEFINES = [
            'ENABLE_DASSERT=1',
            'ENABLE_ION_REMOTE=1',
            'ENABLE_LOGGING=1',
            ('CHECK_GL_ERRORS', 'true'),
        ],
    )

# Add Ion settings.
IonSetup(env, mode = 'opt' if optimize else 'dbg', root_dir = '/local/inst/ion')

# Configuration for using OpenXR. This has to be last or there will be a
# segmentation fault before main() is called.
env.ParseConfig('pkg-config openxr --cflags --libs')

# -----------------------------------------------------------------------------
# Building targets.
# -----------------------------------------------------------------------------

Export(['env', 'build_dir'])
SConscript('submodules/SConscript')

sources = [f'{build_dir}/{source}' for source in [
    'gfx.cpp',
    'vr.cpp',
    'xrtest.cpp',
]]

env.Program(f'{build_dir}/glfwtest', [f'{build_dir}/glfwtest.cpp'] + sources)
