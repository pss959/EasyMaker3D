from brief     import Brief
from ionsetup  import IonSetup
from stringify import Stringify

# -----------------------------------------------------------------------------
# Configuration.
# -----------------------------------------------------------------------------

# Set this to True or False to build debug or optimized.
optimize = False

# Set this to True or False for brief output.
brief = False # True

# All build products go into this directory.
build_dir = 'build'

# -----------------------------------------------------------------------------
# Source file definitions.
# -----------------------------------------------------------------------------

# These are relative to 'src' subdirectory.
lib_sources = [
    'Application.cpp',
    'Controller.cpp',
    'GLFWViewer.cpp',
    'Loader.cpp',
    'LogHandler.cpp',
    'Parser/Field.cpp',
    'Parser/Parser.cpp',
    'Renderer.cpp',
    'Scene.cpp',
    'Util.cpp',
    'VR/OpenXRVR.cpp',
    'VR/OpenXRVRBase.cpp',
    'VR/OpenXRVRInput.cpp',
    'ViewHandler.cpp',
]

# These are relative to 'src/tests' subdirectory.
test_sources = [
    'EventTest.cpp',
    'FlagsTest.cpp',
    'HandTest.cpp',
    'ParserTest.cpp',

    'TestMain.cpp',  # main() function that runs all tests.
]

# -----------------------------------------------------------------------------
# Base environment setup.
# -----------------------------------------------------------------------------

# Convenience to add double quotes to a CPPDEFINE.
def QuoteDef(s):
    return '"\\"' + s + '\\""'

# Send all build products to build_dir.
VariantDir(build_dir, 'src', duplicate = 0)

# Flags used for both compiler and linker.
common_flags = [
    '--std=c++17',
    '-Wall',
    '-Werror',
]

base_env = Environment(
    BUILD_DIR     = build_dir,
    CPPPATH = [
        "#/src",
        '#/submodules/magic_enum/include',
    ],
    CPPDEFINES = [('RESOURCE_DIR', QuoteDef(Dir('#/resources').abspath))],
    CXXFLAGS  = common_flags,
    LINKFLAGS = common_flags,
)

# Create SCons's database file in the build directory for easy cleanup.
base_env.SConsignFile('$BUILD_DIR/sconsign.dblite')

# Shorten compile/link lines for clarity
if brief:
    Brief(base_env)

# Specialize for debug or optimized modes.
if optimize:
    base_env.Append(
        CXXFLAGS  = ['-O3'],
        LINKFLAGS = ['-O3', '-Wl,--strip-all'],
        CPPDEFINES = [('CHECK_GL_ERRORS', 'false')],
    )
else:
    base_env.Append(
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

# Add Ion settings.
IonSetup(base_env, mode='opt' if optimize else 'dbg',
         root_dir='/local/inst/ion', use_shared=True)

pkg_config_str = f'pkg-config {" ".join(packages)} --cflags --libs'
base_env.ParseConfig(pkg_config_str)

# -----------------------------------------------------------------------------
# 'reg_env' is the regular environment, and 'cov_env' is the environment used
# to generate code coverage. Both are derived from base_env.
# -----------------------------------------------------------------------------

reg_env = base_env.Clone()
cov_env = base_env.Clone()

cov_env.Append(
    # Use a different suffix for compiled sources so that they can be in the
    # same directory as regular compiled sources.
    SHOBJSUFFIX = '_cov.os',
    CXXFLAGS  = ['--coverage' ],
    LINKFLAGS = ['--coverage' ],
)

# -----------------------------------------------------------------------------
# Building IMakerVR library so tests can link against it. It has to be a shared
# library so that it can link against other shared libraries.
# -----------------------------------------------------------------------------

placed_lib_sources = [f'$BUILD_DIR/{source}' for source in lib_sources]

# Build regular and coverage-enabled object files.
reg_lib_objects = [reg_env.SharedObject(source=source)
                   for source in placed_lib_sources]
cov_lib_objects = [cov_env.SharedObject(source=source)
                   for source in placed_lib_sources]

reg_lib = reg_env.SharedLibrary('$BUILD_DIR/imakervr',     reg_lib_objects)
cov_lib = cov_env.SharedLibrary('$BUILD_DIR/imakervr_cov', cov_lib_objects)

# -----------------------------------------------------------------------------
# Building IMakerVR application. No need for a coverage-enabled version.
# -----------------------------------------------------------------------------

# Build the application.
app_name = 'imakervr'
app = reg_env.Program(f'$BUILD_DIR/{app_name}',
                      ['$BUILD_DIR/main.cpp'], LIBS=[reg_lib])
reg_env.Default(app)

# -----------------------------------------------------------------------------
# Building tests.
# -----------------------------------------------------------------------------

# Create two test environments.
reg_test_env = reg_env.Clone()
cov_test_env = cov_env.Clone()

# Add the regular or coverage-enabled IMakerVR library.
reg_test_env.Append(LIBS = ['imakervr'])
cov_test_env.Append(LIBS = ['imakervr_cov'])

# Add necessary testing infrastructure.
for env in [reg_test_env, cov_test_env]:
    env.Append(
        CPPPATH = ['#submodules/googletest/googletest/include'],
        LIBPATH = ['#$BUILD_DIR', '$BUILD_DIR/googletest'],
        LIBS    = ['gtest', 'pthread'],
        RPATH   = [Dir('#$BUILD_DIR').abspath,
                   Dir('#$BUILD_DIR/googletest').abspath],
    )

# Build test object files for both environments.
def BuildTests(env, test_app_name):
    placed_sources = [f'$BUILD_DIR/tests/{source}' for source in test_sources]
    objects = [env.SharedObject(source=source) for source in placed_sources]

    # Build all unit tests into a single program.
    return env.Program(f'#$BUILD_DIR/tests/{test_app_name}', objects)

base_env.Alias('RegTests', BuildTests(reg_test_env, 'RegUnitTest'))
base_env.Alias('CovTests', BuildTests(cov_test_env, 'CovUnitTest'))

# placed_test_sources = [f'$BUILD_DIR/tests/{source}' for source in test_sources]
# reg_test_objects = [reg_test_env.SharedObject(source=source)
#                     for source in placed_test_sources]
# cov_test_objects = [cov_test_env.SharedObject(source=source)
#                     for source in placed_test_sources]

# # Build all unit tests into a single program.
# reg_unit_test = reg_test_env.Program(f'#$BUILD_DIR/tests/UnitTest',
#                                      reg_test_objects)
# cov_unit_test = cov_env.Program(f'#$BUILD_DIR/tests/UnitTest_cov',
#                                 cov_test_objects)

# env.Alias('RegTests', reg_unit_test)
# env.Alias('CovTests', cov_unit_test)

#test_args = ''
#env.Alias('Runtests', unit_test, f'$SOURCE {test_args}')

# # Generating coverage results.
# gen_coverage = test_cov_env.Command(
#     '#$BUILD_DIR/coverage/index.html', unit_test_cov,
#     [   # Run the test.
#         '$BUILD_DIR/tests/UnitTest_cov {test_args}',
#         # Generate coverage .
#         'mkdir -p $BUILD_DIR/coverage',
#         'gcov --stdout $BUILD_DIR/tests/TestMain_cov.os > $BUILD_DIR/coverage/coverage.gcov',
#      ])

# env.Alias('Coverage', gen_coverage)

# Make sure run-test target are always considered out of date.
# env.AlwaysBuild(['Runtests', 'RuntestsCov'])

# -----------------------------------------------------------------------------
# Include submodule and doc build files.
# -----------------------------------------------------------------------------

Export('build_dir')
SConscript('submodules/SConscript')
doc = SConscript('InternalDoc/SConscript')

# -----------------------------------------------------------------------------
# Aliases.
# -----------------------------------------------------------------------------

reg_env.Alias('Doc', [doc])
reg_env.Alias('All', [app, 'Doc'])
