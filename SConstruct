from brief import Brief
from os    import environ

# -----------------------------------------------------------------------------
# Configuration.
# -----------------------------------------------------------------------------

AddOption('--mode', dest='mode', type='string', nargs=1, action='store',
          default='dbg', metavar='dbg|opt', help='optimized/debug mode')

# Set this to True or False to build debug or optimized.
optimize = GetOption('mode') == 'opt'

# Set this to True or False for brief output.
brief = True

# All build products go into this directory.
opt_or_dbg = 'opt' if optimize else 'dbg'
build_dir = f'build/{opt_or_dbg}'

# Documentation is not mode-dependent, so it just goes under build.
doc_build_dir = 'build'

# -----------------------------------------------------------------------------
# Source file definitions.
# -----------------------------------------------------------------------------

# These are relative to 'src' subdirectory.
lib_sources = [
    'ActionMap.cpp',
    'AppInfo.cpp',
    'Application.cpp',
    'CoordConv.cpp',
    'Defaults.cpp',
    'Event.cpp',
    'Procedural.cpp',
    'RadialMenuInfo.cpp',
    'RegisterTypes.cpp',
    'Renderer.cpp',
    'SceneContext.cpp',
    'SelPath.cpp',
    'SessionState.cpp',
    'Settings.cpp',
    'UnitConversion.cpp',

    'Commands/ChangeBevelCommand.cpp',
    'Commands/ChangeCSGOperationCommand.cpp',
    'Commands/ChangeColorCommand.cpp',
    'Commands/ChangeComplexityCommand.cpp',
    'Commands/ChangeCylinderCommand.cpp',
    'Commands/ChangeEdgeTargetCommand.cpp',
    'Commands/ChangeImportedModelCommand.cpp',
    'Commands/ChangeNameCommand.cpp',
    'Commands/ChangeOrderCommand.cpp',
    'Commands/ChangePointTargetCommand.cpp',
    'Commands/CombineCommand.cpp',
    'Commands/CommandList.cpp',
    'Commands/ConvertBevelCommand.cpp',
    'Commands/CreateCSGModelCommand.cpp',
    'Commands/CreateHullModelCommand.cpp',
    'Commands/CreateImportedModelCommand.cpp',
    'Commands/CreateModelCommand.cpp',
    'Commands/CreatePrimitiveModelCommand.cpp',
    'Commands/CreateTextModelCommand.cpp',
    'Commands/MultiModelCommand.cpp',
    'Commands/RotateCommand.cpp',
    'Commands/ScaleCommand.cpp',
    'Commands/SingleModelCommand.cpp',
    'Commands/TranslateCommand.cpp',

    'Executors/ChangeBevelExecutor.cpp',
    'Executors/ChangeCSGExecutor.cpp',
    'Executors/ChangeColorExecutor.cpp',
    'Executors/ChangeComplexityExecutor.cpp',
    'Executors/ChangeCylinderExecutor.cpp',
    'Executors/ChangeImportedExecutor.cpp',
    'Executors/ChangeNameExecutor.cpp',
    'Executors/ChangeOrderExecutor.cpp',
    'Executors/CombineExecutorBase.cpp',
    'Executors/ConvertBevelExecutor.cpp',
    'Executors/ConvertExecutorBase.cpp',
    'Executors/CreateCSGExecutor.cpp',
    'Executors/CreateHullExecutor.cpp',
    'Executors/CreateImportedExecutor.cpp',
    'Executors/CreatePrimitiveExecutor.cpp',
    'Executors/CreateTextExecutor.cpp',
    'Executors/EdgeTargetExecutor.cpp',
    'Executors/Executor.cpp',
    'Executors/InitExecutors.cpp',
    'Executors/ModelExecutorBase.cpp',
    'Executors/PointTargetExecutor.cpp',
    'Executors/RotateExecutor.cpp',
    'Executors/ScaleExecutor.cpp',
    'Executors/TranslateExecutor.cpp',

    'Feedback/AngularFeedback.cpp',
    'Feedback/FindFeedback.cpp',
    'Feedback/LinearFeedback.cpp',
    'Feedback/TooltipFeedback.cpp',

    'Handlers/BoardHandler.cpp',
    'Handlers/ControllerHandler.cpp',
    'Handlers/InspectorHandler.cpp',
    'Handlers/LogHandler.cpp',
    'Handlers/MainHandler.cpp',
    'Handlers/ShortcutHandler.cpp',
    'Handlers/ViewHandler.cpp',

    'IO/Reader.cpp',
    'IO/STLReader.cpp',

    'Items/Board.cpp',
    'Items/Controller.cpp',
    'Items/Frame.cpp',
    'Items/GripGuide.cpp',
    'Items/Grippable.cpp',
    'Items/Icon.cpp',
    'Items/Inspector.cpp',
    'Items/PaneBackground.cpp',
    'Items/PaneBorder.cpp',
    'Items/PrecisionControl.cpp',
    'Items/RadialMenu.cpp',
    'Items/Shelf.cpp',

    'Managers/ActionManager.cpp',
    'Managers/AnimationManager.cpp',
    'Managers/ClipboardManager.cpp',
    'Managers/ColorManager.cpp',
    'Managers/CommandManager.cpp',
    'Managers/FeedbackManager.cpp',
    'Managers/InstanceManager.cpp',
    'Managers/NameManager.cpp',
    'Managers/PanelManager.cpp',
    'Managers/SelectionManager.cpp',
    'Managers/SessionManager.cpp',
    'Managers/SettingsManager.cpp',
    'Managers/TargetManager.cpp',
    'Managers/ToolManager.cpp',

    'Math/Animation.cpp',
    'Math/Bevel.cpp',
    'Math/Beveler.cpp',
    'Math/CGALInternal.cpp',
    'Math/Curves.cpp',
    'Math/Intersection.cpp',
    'Math/Linear.cpp',
    'Math/MeshBuilding.cpp',
    'Math/MeshCombining.cpp',
    'Math/MeshUtils.cpp',
    'Math/MeshValidation.cpp',
    'Math/Point3fMap.cpp',
    'Math/PolyMesh.cpp',
    'Math/PolyMeshBuilder.cpp',
    'Math/PolyMeshMerging.cpp',
    'Math/Polygon.cpp',
    'Math/PolygonBuilder.cpp',
    'Math/Profile.cpp',
    'Math/TextUtils.cpp',
    'Math/Triangulation.cpp',
    'Math/Types.cpp',
    'Math/VertexRing.cpp',

    'Models/BeveledModel.cpp',
    'Models/BoxModel.cpp',
    'Models/CSGModel.cpp',
    'Models/CombinedModel.cpp',
    'Models/ConvertedModel.cpp',
    'Models/CylinderModel.cpp',
    'Models/HullModel.cpp',
    'Models/ImportedModel.cpp',
    'Models/Model.cpp',
    'Models/ParentModel.cpp',
    'Models/RevSurfModel.cpp',
    'Models/RootModel.cpp',
    'Models/SphereModel.cpp',
    'Models/TextModel.cpp',
    'Models/TorusModel.cpp',

    'Panels/ActionPanel.cpp',
    'Panels/BevelToolPanel.cpp',
    'Panels/DialogPanel.cpp',
    'Panels/FilePanel.cpp',
    'Panels/HelpPanel.cpp',
    'Panels/ImportToolPanel.cpp',
    'Panels/InfoPanel.cpp',
    'Panels/NameToolPanel.cpp',
    'Panels/Panel.cpp',
    'Panels/RadialMenuPanel.cpp',
    'Panels/SessionPanel.cpp',
    'Panels/SettingsPanel.cpp',
    'Panels/TextToolPanel.cpp',
    'Panels/TreePanel.cpp',

    'Panes/BoxPane.cpp',
    'Panes/ButtonPane.cpp',
    'Panes/CheckboxPane.cpp',
    'Panes/ClipPane.cpp',
    'Panes/DropdownPane.cpp',
    'Panes/ContainerPane.cpp',
    'Panes/GridPane.cpp',
    'Panes/IconPane.cpp',
    'Panes/ImagePane.cpp',
    'Panes/LabeledSliderPane.cpp',
    'Panes/Pane.cpp',
    'Panes/ProfilePane.cpp',
    'Panes/RadioButtonPane.cpp',
    'Panes/ScrollingPane.cpp',
    'Panes/SliderPane.cpp',
    'Panes/SwitcherPane.cpp',
    'Panes/TextInputPane.cpp',
    'Panes/TextPane.cpp',

    'Parser/Field.cpp',
    'Parser/Object.cpp',
    'Parser/Parser.cpp',
    'Parser/Registry.cpp',
    'Parser/Scanner.cpp',
    'Parser/ValueWriter.cpp',
    'Parser/Writer.cpp',

    'SG/Box.cpp',
    'SG/Cylinder.cpp',
    'SG/Ellipsoid.cpp',
    'SG/FileImage.cpp',
    'SG/Gantry.cpp',
    'SG/Image.cpp',
    'SG/ImportedShape.cpp',
    'SG/Init.cpp',
    'SG/Intersector.cpp',
    'SG/IonContext.cpp',
    'SG/LayoutOptions.cpp',
    'SG/LightingPass.cpp',
    'SG/Line.cpp',
    'SG/Material.cpp',
    'SG/Node.cpp',
    'SG/NodePath.cpp',
    'SG/Object.cpp',
    'SG/PointLight.cpp',
    'SG/PolyLine.cpp',
    'SG/Polygon.cpp',
    'SG/PrimitiveShape.cpp',
    'SG/ProceduralImage.cpp',
    'SG/Rectangle.cpp',
    'SG/RegularPolygon.cpp',
    'SG/RenderPass.cpp',
    'SG/Sampler.cpp',
    'SG/Scene.cpp',
    'SG/Search.cpp',
    'SG/ShaderProgram.cpp',
    'SG/ShaderSource.cpp',
    'SG/ShadowPass.cpp',
    'SG/Shape.cpp',
    'SG/StateTable.cpp',
    'SG/SubImage.cpp',
    'SG/TextNode.cpp',
    'SG/Texture.cpp',
    'SG/Torus.cpp',
    'SG/Tracker.cpp',
    'SG/TriMeshShape.cpp',
    'SG/Uniform.cpp',
    'SG/UniformBlock.cpp',
    'SG/UniformDef.cpp',
    'SG/VRCamera.cpp',
    'SG/WindowCamera.cpp',

    'Targets/EdgeTarget.cpp',
    'Targets/PointTarget.cpp',

    'Tools/BevelTool.cpp',
    'Tools/ColorTool.cpp',
    'Tools/ComplexityTool.cpp',
    'Tools/CylinderTool.cpp',
    'Tools/FindTools.cpp',
    'Tools/ImportTool.cpp',
    'Tools/NameTool.cpp',
    'Tools/PanelTool.cpp',
    'Tools/PassiveTool.cpp',
    'Tools/RotationTool.cpp',
    'Tools/ScaleTool.cpp',
    'Tools/TextTool.cpp',
    'Tools/Tool.cpp',
    'Tools/TranslationTool.cpp',

    'Util/Delay.cpp',
    'Util/FilePath.cpp',
    'Util/General.cpp',
    'Util/KLog.cpp',
    'Util/Read.cpp',
    'Util/StackTrace.cpp',
    'Util/String.cpp',
    'Util/URL.cpp',
    'Util/UTime.cpp',

    'Viewers/GLFWViewer.cpp',
    'Viewers/VRViewer.cpp',

    'VR/VRBase.cpp',
    'VR/VRContext.cpp',
    'VR/VRInput.cpp',
    'VR/VRStructs.cpp',

    'Widgets/DiscWidget.cpp',
    'Widgets/EdgeTargetWidget.cpp',
    'Widgets/GenericWidget.cpp',
    'Widgets/IconSwitcherWidget.cpp',
    'Widgets/IconWidget.cpp',
    'Widgets/PointTargetWidget.cpp',
    'Widgets/PushButtonWidget.cpp',
    'Widgets/ScaleWidget.cpp',
    'Widgets/Slider1DWidget.cpp',
    'Widgets/Slider2DWidget.cpp',
    'Widgets/SliderWidgetBase.cpp',
    'Widgets/SphereWidget.cpp',
    'Widgets/StageWidget.cpp',
    'Widgets/TargetWidgetBase.cpp',
    'Widgets/Widget.cpp',
]

# Source files that are always optimized due to performance issues.
slow_lib_sources = [
    'Math/MeshValidation.cpp',
]

# Source files that needs special treatment on Windows because of size.
big_lib_sources = [
    'Math/MeshCombining.cpp',
]

# Add debug-only sources.
if not optimize:
    lib_sources += [
        'Debug/Dump3dv.cpp',
        'Debug/Print.cpp',
        'Panels/TestPanel.cpp',
    ]

# These are relative to 'src/tests' subdirectory.
test_sources = [
    'AssertTest.cpp',
    'BevelerTest.cpp',
    'BoundsTest.cpp',
    'CloneTest.cpp',
    'ColorTest.cpp',
    'CoordConvTest.cpp',
    'DelayTest.cpp',
    'DimensionalityTest.cpp',
    'EnumTest.cpp',
    'EventTest.cpp',
    'FeedbackManagerTest.cpp',
    'FilePathTest.cpp',
    'FlagsTest.cpp',
    'FrustumTest.cpp',
    'HandTest.cpp',
    'IntersectorTest.cpp',
    'LinearTest.cpp',
    'MeshBuildingTest.cpp',
    'MeshUtilsTest.cpp',
    'ModelTest.cpp',
    'NameManagerTest.cpp',
    'NodePathTest.cpp',
    'NodeTest.cpp',
    'ParserTest.cpp',
    'PlaneTest.cpp',
    'PolyMeshTest.cpp',
    'PolygonTest.cpp',
    'ReadFileTest.cpp',
    'ReaderTest.cpp',
    'RegistryTest.cpp',
    'STLTest.cpp',
    'SceneTestBase.cpp',
    'SearchTest.cpp',
    'StringTest.cpp',
    'TestBase.cpp',
    'TextUtilsTest.cpp',
    'TriangulationTest.cpp',
    'UTimeTest.cpp',
    'UtilTest.cpp',

    'TestMain.cpp',  # main() function that runs all tests.
]

# -----------------------------------------------------------------------------
# Platform-specific environment setup.
# -----------------------------------------------------------------------------

platform = str(Platform())
if platform.startswith('win'):
    platform = "windows"
elif platform == 'darwin':
    platform = "mac"
else:
    platform = 'linux'

if platform == 'windows':
    base_env = Environment(tools = ['mingw'])
else:
    base_env = Environment()

# -----------------------------------------------------------------------------
# Base environment setup.
# -----------------------------------------------------------------------------

# Convenience to add double quotes to a CPPDEFINE.
def QuoteDef(s):
    return '"\\"' + s + '\\""'

# Send all build products to build_dir.
VariantDir(build_dir, 'src', duplicate=False)

base_env.Replace(
    BUILD_DIR = build_dir,
    CPPPATH = [
        "#/src",
        "#/ionsrc/Ion",
        '#/ionsrc/Ion/ion/port/override',
        "#/ionsrc/Ion/third_party/google",
        '#/submodules/magic_enum/include',
        '#/submodules/third_party',
        '#/submodules/third_party/absl',
        '#/submodules/third_party/google',
        '#/submodules/third_party/image_compression',
    ],
    CPPDEFINES = [
        ('RESOURCE_DIR',  QuoteDef(Dir('#/resources').abspath)),
        ('TEST_DATA_DIR', QuoteDef(Dir('#/src/Tests/Data').abspath)),

        # Required for Ion.
        'ARCH_K8',
        'OPENCTM_NO_CPP',
        ('ION_ARCH_X86_64', '1'),
        ('ION_NO_RTTI', '0'),
    ],
    LIBPATH = ['$BUILD_DIR'],
    RPATH   = [Dir('#$BUILD_DIR').abspath],
    LIBS    = ['ionshared', 'mpfr', 'gmp'],   # Required for CGAL.
)

if not optimize:
    base_env.Append(
        CPPDEFINES = [
            '_DEBUG',
            ('DEBUG', '1'),
            ('ION_DEBUG', '1'),
            # ('ION_TRACK_SHAREABLE_REFERENCES', '1'),
        ],
    )

# Shorten compile/link lines for clarity
if brief:
    Brief(base_env)

# Create SCons's database file in the build directory for easy cleanup.
base_env.SConsignFile('$BUILD_DIR/sconsign.dblite')

# -----------------------------------------------------------------------------
# Platform-specific environment setup.
# -----------------------------------------------------------------------------

# Platform-specific variables, compiler, and linker flags.
if platform == 'windows':
    base_env.Append(
        CPPDEFINES = [
            'COMPILER_HAS_RVALUEREF',
            'NOGDI',                # Disables "ERROR" macro.
            'NOMINMAX',
            'PRAGMA_SUPPORTED',
            'UNICODE=1',
            'WIN32_LEAN_AND_MEAN=1',
            '_CRT_SECURE_NO_DEPRECATE',
            '_USE_MATH_DEFINES',    # Enables M_PI.
            '_WIN32',
            'GLFW_DLL',             # Required by glfw3.
            ('ION_API', ''),
            ('ION_APIENTRY', 'APIENTRY'),
            ('ION_PLATFORM_WINDOWS', '1'),
            ('OS_WINDOWS' 'OS_WINDOWS'),
        ],
    )
    # Note: the "-O1" keeps big files from choking on Windows ("string table
    # overflow", "file too big").
    big_cflags = ['-O1']
    pkg_config_opts = '--static'
    run_program = f'bin\\runprogram.bat {opt_or_dbg}'

elif platform == 'linux':
    base_env.Append(
        CPPDEFINES = [
            ('ION_API', ''),
            ('ION_APIENTRY', ''),
            ('ION_PLATFORM_LINUX', '1'),
        ],
        LIBS = ['GLX', 'GLU', 'GL', 'X11', 'dl', 'pthread', 'm'],
    )
    big_cflags = []
    pkg_config_opts = ''
    run_program = ''

common_flags = [
    '--std=c++17',
    '-Wall',
    '-Werror',
    '-Wextra',
    '-Wmissing-declarations',
    '-Wold-style-cast',
    '-Wuninitialized',
    '-Wno-unused-parameter',    # This causes problems in Ion headers:
    '-Wno-strict-aliasing',     # Ion has issues with this.
]

# Specialize for debug or optimized modes.
if optimize:
    base_env.Append(
        CXXFLAGS   = common_flags + ['-O3'],
        LINKFLAGS  = common_flags + ['-O3', '-Wl,--strip-all'],
        CPPDEFINES = [('CHECK_GL_ERRORS', 'false')],
    )
else:
    base_env.Append(
        CXXFLAGS   = common_flags + ['-g'],
        LINKFLAGS  = common_flags + ['-g'],
        CPPDEFINES = [
            'ENABLE_DASSERT=1',
            'ENABLE_ION_REMOTE=1',
            'ENABLE_LOGGING=1',
            ('CHECK_GL_ERRORS', 'true'),
            # This allows valgrind to work on the debug executables.
            'CGAL_DISABLE_ROUNDING_MATH_CHECK',
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

package_str = ' '.join(packages)
base_env.ParseConfig(
    f'pkg-config {pkg_config_opts} {package_str} --cflags --libs')

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

def BuildObject(env, source):
    if source in big_lib_sources:
        extra_flags = big_cflags
    elif source in slow_lib_sources:
        extra_flags = ['-O3']
    else:
        extra_flags = []
    flags = env['CXXFLAGS'] + extra_flags
    return env.SharedObject(source=f'$BUILD_DIR/{source}', CXXFLAGS=flags)

# Build regular and coverage-enabled object files.
reg_lib_objects = [BuildObject(reg_env, source) for source in lib_sources]
cov_lib_objects = [BuildObject(cov_env, source) for source in lib_sources]

reg_lib = reg_env.SharedLibrary('$BUILD_DIR/imakervr',     reg_lib_objects)
cov_lib = cov_env.SharedLibrary('$BUILD_DIR/imakervr_cov', cov_lib_objects)

reg_env.Alias('Lib', reg_lib)

# -----------------------------------------------------------------------------
# Building IMakerVR and related applications. No need for a coverage-enabled
# version.
# -----------------------------------------------------------------------------

# Build the applications.
apps = ['imakervr', 'printtypes', 'nodeviewer']

app_env = reg_env.Clone()
app_env.Append(LIBS=['imakervr', 'docopt'])
imakervr=None
for app_name in apps:
    if platform == 'windows':
        linkflags = ['-static']
    else:
        linkflags = []
    app = app_env.Program(f'$BUILD_DIR/Apps/{app_name}',
                          [f'$BUILD_DIR/Apps/{app_name}.cpp'],
                          LINKFLAGS=linkflags)
    app_env.Default(app)
    app_env.Alias('Apps', app)

    # Main app is special
    if app_name == 'imakervr':
        imakervr = app

# -----------------------------------------------------------------------------
# Running IMakerVR application.
# -----------------------------------------------------------------------------

# Create an execution environment that has all the regular environment
# variables so that the X11 display works.
exec_env = reg_env.Clone(ENV = environ)

exec_env.Alias('RunApp', imakervr,
               f'{run_program} $SOURCE ')  # End space seems to be required.

# Make sure run target is always considered out of date.
exec_env.AlwaysBuild('RunApp')

# -----------------------------------------------------------------------------
# Building tests.
# -----------------------------------------------------------------------------

# Create two test environments with all the regular environment variables.
reg_test_env = reg_env.Clone(ENV = environ)
cov_test_env = cov_env.Clone(ENV = environ)

# Add the regular or coverage-enabled IMakerVR library.
reg_test_env.Append(LIBS = ['imakervr'])
cov_test_env.Append(LIBS = ['imakervr_cov'])

# Add necessary testing infrastructure.
for env in [reg_test_env, cov_test_env]:
    env.Append(
        CPPPATH = ['#submodules/googletest/googletest/include'],
        LIBPATH = ['#$BUILD_DIR', '$BUILD_DIR/googletest'],
        LIBS    = ['gtest', 'pthread'],
        RPATH   = [Dir('#$BUILD_DIR/googletest').abspath],
    )

# Build test object files for both environments.
def BuildTests(env, test_app_name):
    defines = env['CPPDEFINES'] + [('IN_UNIT_TEST', 1)]
    placed_sources = [f'$BUILD_DIR/Tests/{source}' for source in test_sources]
    objects = [env.SharedObject(source=source, CPPDEFINES=defines)
               for source in placed_sources]

    # Build all unit tests into a single program.
    return (objects, env.Program(f'#$BUILD_DIR/Tests/{test_app_name}', objects))

(reg_test_objects, reg_test) = BuildTests(reg_test_env, 'RegUnitTest')
(cov_test_objects, cov_test) = BuildTests(cov_test_env, 'CovUnitTest')

reg_env.Alias('RegTests', reg_test)
cov_env.Alias('CovTests', cov_test)

# -----------------------------------------------------------------------------
# Running tests.
# -----------------------------------------------------------------------------

test_filter = ARGUMENTS.get('TESTFILTER')
test_args = ('--gtest_filter="%s"' % test_filter) if test_filter else ''

reg_env.Alias('RunRegTests', reg_test, f'{run_program} $SOURCE {test_args}')
cov_env.Alias('RunCovTests', cov_test, f'{run_program} $SOURCE {test_args}')

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
# Include Ion, submodule, resources, and doc build files.
# -----------------------------------------------------------------------------

Export('brief', 'build_dir', 'doc_build_dir', 'optimize', 'platform')

reg_env.Alias('Icons', SConscript('resources/SConscript'))
SConscript('submodules/SConscript')
doc = SConscript('InternalDoc/SConscript')
ion = SConscript('ionsrc/Ion/SConscript', variant_dir = f'{build_dir}/Ion',
                 duplicate=False)

# Applications depend on the icons.
reg_env.Depends('Apps', 'Icons')

# -----------------------------------------------------------------------------
# Other Aliases.
# -----------------------------------------------------------------------------

reg_env.Alias('Doc', [doc])
reg_env.Alias('All', [app, 'Doc'])
reg_env.Alias('Ion', ion)
