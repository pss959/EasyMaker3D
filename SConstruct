# -----------------------------------------------------------------------------
# Configuration.
# -----------------------------------------------------------------------------

# Set this to True or False to build debug or optimized.
optimize = False

# Set this to True or False for brief output.
brief = True

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

# Convenience to add double quotes to a CPPDEFINE.
def QuoteDef(s):
    return '"\\"' + s + '\\""'

common_flags = [
    '--std=c++17',
    '-Wall',
]

env = Environment(
    BUILD_DIR     = build_dir,
    ABS_BUILD_DIR = Dir(build_dir).abspath,

    CPPPATH = [
        "#/src",
        '#/submodules/magic_enum/include',
    ],
    CPPDEFINES = [('RESOURCE_DIR', QuoteDef(Dir('#/resources').abspath))],
    CXXFLAGS  = common_flags,
    LINKFLAGS = common_flags,
    LIBS = ['boost_python39'],
)

# Shorten compile/link lines for clarity
if brief:
    Brief(env)

# Create SCons's database file in the build directory for easy cleanup.
env.SConsignFile('$BUILD_DIR/sconsign.dblite')

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

packages = [
    'freetype2',
    'glfw3',
    'jsoncpp',
    'libjpeg',
    'minizip',
    'openxr',
    'stb',
    'tinyxml2',
    'zlib',
]

# Add Ion settings after other libraries.
IonSetup(env, mode='opt' if optimize else 'dbg', root_dir='/local/inst/ion',
         use_shared=True)

pkg_config_str = f'pkg-config {" ".join(packages)} --cflags --libs'
env.ParseConfig(pkg_config_str)

# -----------------------------------------------------------------------------
# Building targets.
# -----------------------------------------------------------------------------

sources = [f'$BUILD_DIR/{source}' for source in [
    'Application.cpp',
    'Controller.cpp',
    'GLFWViewer.cpp',
    'Loader.cpp',
    'LogHandler.cpp',
    'Parser.cpp',
    'Renderer.cpp',
    'Scene.cpp',
    'Util.cpp',
    'VR/OpenXRVR.cpp',
    'VR/OpenXRVRBase.cpp',
    'VR/OpenXRVRInput.cpp',
    'ViewHandler.cpp',
]]

app_name = 'imakervr'
app = env.Program(f'$BUILD_DIR/{app_name}',
                  ['$BUILD_DIR/main.cpp'] + sources)

Export('env')
SConscript('submodules/SConscript')
SConscript('src/tests/SConscript')
SConscript('InternalDoc/SConscript')

env.Default(app)

# -----------------------------------------------------------------------------
# Aliases.
# -----------------------------------------------------------------------------

env.Alias('All', [app, 'Doc'])
