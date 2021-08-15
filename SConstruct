from brief     import Brief
from ionsetup  import IonSetup
from os        import environ
from stringify import Stringify

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
# Source file definitions.
# -----------------------------------------------------------------------------

# These are relative to 'src' subdirectory.
lib_sources = [
    # 'Application.cpp',
    'Controller.cpp',
    # 'GLFWViewer.cpp',
    'LogHandler.cpp',
    'Renderer.cpp',
    'ShortcutHandler.cpp',
    'View.cpp',
    'ViewHandler.cpp',

    'Graph/Camera.cpp',
    'Graph/Image.cpp',
    'Graph/Node.cpp',
    'Graph/Sampler.cpp',
    'Graph/Scene.cpp',
    'Graph/ShaderProgram.cpp',
    'Graph/Shape.cpp',
    'Graph/Texture.cpp',
    'Graph/Transform.cpp',

    'Input/Conversion.cpp',
    'Input/Extractor.cpp',
    'Input/Reader.cpp',
    'Input/Tracker.cpp',

    'Managers/NameManager.cpp',

    'Parser/Field.cpp',
    'Parser/Parser.cpp',
    'Parser/Visitor.cpp',

    'Util/Read.cpp',
    'Util/Search.cpp',
    'Util/String.cpp',

    # 'VR/OpenXRVR.cpp',
    # 'VR/OpenXRVRBase.cpp',
    # 'VR/OpenXRVRInput.cpp',
]

# These are relative to 'src/tests' subdirectory.
test_sources = [
    'EnumTest.cpp',
    'EventTest.cpp',
    'FilePathTest.cpp',
    'HandTest.cpp',
    'InputTest.cpp',
    'NameManagerTest.cpp',
    'ParserTest.cpp',
    'StringTest.cpp',
    'TimeTest.cpp',

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
    LIBPATH   = ['$BUILD_DIR'],
    RPATH     = [Dir('#$BUILD_DIR').abspath],
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
    CXXFLAGS  = ['--coverage' ],
    LINKFLAGS = ['--coverage' ],
)
# Use a different suffix for compiled sources so that they can be in the
# same directory as regular compiled sources.
cov_env.Replace(SHOBJSUFFIX = '_cov.os')

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

reg_env.Alias('Lib', reg_lib)

# -----------------------------------------------------------------------------
# Building IMakerVR application. No need for a coverage-enabled version.
# -----------------------------------------------------------------------------

# Build the application.
app_name = 'imakervr'
app = reg_env.Program(f'$BUILD_DIR/{app_name}',
                      ['$BUILD_DIR/main.cpp'], LIBS=['imakervr'])
reg_env.Default(app)

# -----------------------------------------------------------------------------
# Running IMakerVR application.
# -----------------------------------------------------------------------------

# Create an execution environment that has all the regular environment
# variables so that the X11 display works.
exec_env = reg_env.Clone(ENV = environ)

exec_env.Alias('RunApp', app, '$SOURCE ')  # Space is necessary for some reason.

# Make sure run target is always considered out of date.
exec_env.AlwaysBuild('RunApp')

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
        LIBS    = ['gtest', 'boost_filesystem', 'pthread'],
        RPATH   = [Dir('#$BUILD_DIR/googletest').abspath],
    )

# Build test object files for both environments.
def BuildTests(env, test_app_name):
    placed_sources = [f'$BUILD_DIR/tests/{source}' for source in test_sources]
    objects = [env.SharedObject(source=source) for source in placed_sources]

    # Build all unit tests into a single program.
    return (objects, env.Program(f'#$BUILD_DIR/tests/{test_app_name}', objects))

(reg_test_objects, reg_test) = BuildTests(reg_test_env, 'RegUnitTest')
(cov_test_objects, cov_test) = BuildTests(cov_test_env, 'CovUnitTest')

reg_env.Alias('RegTests', reg_test)
cov_env.Alias('CovTests', cov_test)

# -----------------------------------------------------------------------------
# Running tests.
# -----------------------------------------------------------------------------

test_args = '' # '--gtest_filter="Parser*Constants"'

reg_env.Alias('RunRegTests', reg_test, f'$SOURCE {test_args}')
cov_env.Alias('RunCovTests', cov_test, f'$SOURCE {test_args}')

# Make sure test run targets are always considered out of date.
reg_test_env.AlwaysBuild('RunRegTests')
cov_test_env.AlwaysBuild('RunCovTests')

# -----------------------------------------------------------------------------
# Generating coverage results.
# -----------------------------------------------------------------------------

# Use all coverage-enabled object files.
cov_objects = cov_lib_objects + cov_test_objects
cov_object_str = ' '.join([obj[0].path for obj in cov_objects])

# Patterns to remove from coverage results.
rm_patterns  =  '"/usr/include/*" "/local/inst/ion/*" "*/submodules/*"'

# Files for original lcov output and filtered output.
lcov_file1   =  'coverage/coverage.info'
lcov_file2   =  'coverage/coverage_filtered.info'

# Arguments to lcov to capture results and to filter them.
lcov_args1   = f'--capture --directory .. --output-file {lcov_file1}'
lcov_args2   = f'--remove {lcov_file1} {rm_patterns} --output-file {lcov_file2}'

# Arguments to genhtml to produce the HTML results.
genhtml_args = f'--output-directory coverage/html {lcov_file2}'

gen_coverage = cov_test_env.Command(
    '#$BUILD_DIR/coverage/index.html', cov_test,
    [   # Run the test.
        f'$SOURCE {test_args}',
        # Generate coverage .
        'mkdir -p $BUILD_DIR/coverage',
        f'cd $BUILD_DIR ; lcov {lcov_args1} ; lcov {lcov_args2}',
        # Generate HTML from the results .
        f'cd $BUILD_DIR ; genhtml {genhtml_args}',
        ('echo === Coverage results in ' +
         Dir('$BUILD_DIR/coverage/html/index.html').abspath)])

env.Alias('Coverage', gen_coverage)

# -----------------------------------------------------------------------------
# Include submodule and doc build files.
# -----------------------------------------------------------------------------

Export('brief', 'build_dir')
SConscript('submodules/SConscript')
doc = SConscript('InternalDoc/SConscript')

# -----------------------------------------------------------------------------
# Other Aliases.
# -----------------------------------------------------------------------------

reg_env.Alias('Doc', [doc])
reg_env.Alias('All', [app, 'Doc'])
