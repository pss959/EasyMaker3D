from brief import Brief
from os    import environ

# -----------------------------------------------------------------------------
# Configuration.
# -----------------------------------------------------------------------------

APP_NAME            = 'MakerVR'
SESSION_SUFFIX      = '.mvr'
VERSION_STRING      = '0.0.1'
PUBLIC_DOC_BASE_URL = 'https://pss959.github.io/MakerVR-dist/'

AddOption('--mode', dest='mode', type='string', nargs=1, action='store',
          default='dbg', metavar='dbg|opt|rel',
          help='optimized/debug/release mode')
AddOption('--brief', dest='brief', action='store_true',
          default='True', help='Shortened vs. full output')
AddOption('--nobrief', dest='brief', action='store_false',
          default='True', help='Shortened vs. full output')

# Set this to True or False to build debug or optimized.
mode  = GetOption('mode')

# Set this to True or False for brief output.
brief = GetOption('brief')

# All build products go into this directory.
build_dir = f'build/{mode}'

# Documentation is not mode-dependent, so it just goes under build.
doc_build_dir = 'build'

# -----------------------------------------------------------------------------
# Source file definitions.
# -----------------------------------------------------------------------------

# These are all relative to 'src' subdirectory.
lib_sources = [
    'App/Application.cpp',
    'App/CoordConv.cpp',
    'App/RegisterTypes.cpp',
    'App/Renderer.cpp',
    'App/SceneContext.cpp',
    'App/SceneLoader.cpp',
    'App/SelPath.cpp',
    'App/VRContext.cpp',
    'App/VRModelLoader.cpp',

    'Base/ActionMap.cpp',
    'Base/Event.cpp',
    'Base/HelpMap.cpp',
    'Base/Procedural.cpp',
    'Base/Tuning.cpp',
    'Base/VirtualKeyboard.cpp',

    'Commands/ChangeBevelCommand.cpp',
    'Commands/ChangeCSGOperationCommand.cpp',
    'Commands/ChangeClipCommand.cpp',
    'Commands/ChangeColorCommand.cpp',
    'Commands/ChangeComplexityCommand.cpp',
    'Commands/ChangeCylinderCommand.cpp',
    'Commands/ChangeEdgeTargetCommand.cpp',
    'Commands/ChangeImportedModelCommand.cpp',
    'Commands/ChangeMirrorCommand.cpp',
    'Commands/ChangeNameCommand.cpp',
    'Commands/ChangeOrderCommand.cpp',
    'Commands/ChangePointTargetCommand.cpp',
    'Commands/ChangeRevSurfCommand.cpp',
    'Commands/ChangeTextCommand.cpp',
    'Commands/ChangeTorusCommand.cpp',
    'Commands/CombineCommand.cpp',
    'Commands/Command.cpp',
    'Commands/CommandList.cpp',
    'Commands/ConvertBevelCommand.cpp',
    'Commands/ConvertClipCommand.cpp',
    'Commands/ConvertMirrorCommand.cpp',
    'Commands/CopyCommand.cpp',
    'Commands/CreateCSGModelCommand.cpp',
    'Commands/CreateHullModelCommand.cpp',
    'Commands/CreateImportedModelCommand.cpp',
    'Commands/CreateModelCommand.cpp',
    'Commands/CreatePrimitiveModelCommand.cpp',
    'Commands/CreateRevSurfModelCommand.cpp',
    'Commands/CreateTextModelCommand.cpp',
    'Commands/DeleteCommand.cpp',
    'Commands/LinearLayoutCommand.cpp',
    'Commands/MultiModelCommand.cpp',
    'Commands/PasteCommand.cpp',
    'Commands/RadialLayoutCommand.cpp',
    'Commands/RotateCommand.cpp',
    'Commands/ScaleCommand.cpp',
    'Commands/SingleModelCommand.cpp',
    'Commands/TranslateCommand.cpp',

    'Debug/Timer.cpp',

    'Executors/ChangeBevelExecutor.cpp',
    'Executors/ChangeCSGExecutor.cpp',
    'Executors/ChangeClipExecutor.cpp',
    'Executors/ChangeColorExecutor.cpp',
    'Executors/ChangeComplexityExecutor.cpp',
    'Executors/ChangeCylinderExecutor.cpp',
    'Executors/ChangeImportedExecutor.cpp',
    'Executors/ChangeMirrorExecutor.cpp',
    'Executors/ChangeNameExecutor.cpp',
    'Executors/ChangeOrderExecutor.cpp',
    'Executors/ChangeRevSurfExecutor.cpp',
    'Executors/ChangeTextExecutor.cpp',
    'Executors/ChangeTorusExecutor.cpp',
    'Executors/CombineExecutorBase.cpp',
    'Executors/ConvertBevelExecutor.cpp',
    'Executors/ConvertClipExecutor.cpp',
    'Executors/ConvertExecutorBase.cpp',
    'Executors/ConvertMirrorExecutor.cpp',
    'Executors/CopyExecutor.cpp',
    'Executors/CreateCSGExecutor.cpp',
    'Executors/CreateHullExecutor.cpp',
    'Executors/CreateImportedExecutor.cpp',
    'Executors/CreatePrimitiveExecutor.cpp',
    'Executors/CreateRevSurfExecutor.cpp',
    'Executors/CreateTextExecutor.cpp',
    'Executors/DeleteExecutor.cpp',
    'Executors/EdgeTargetExecutor.cpp',
    'Executors/Executor.cpp',
    'Executors/InitExecutors.cpp',
    'Executors/LinearLayoutExecutor.cpp',
    'Executors/ModelExecutorBase.cpp',
    'Executors/PasteExecutor.cpp',
    'Executors/PointTargetExecutor.cpp',
    'Executors/RadialLayoutExecutor.cpp',
    'Executors/RotateExecutor.cpp',
    'Executors/ScaleExecutor.cpp',
    'Executors/TranslateExecutor.cpp',

    'Feedback/AngularFeedback.cpp',
    'Feedback/FindFeedback.cpp',
    'Feedback/LinearFeedback.cpp',
    'Feedback/TooltipFeedback.cpp',

    'Handlers/BoardHandler.cpp',
    'Handlers/ControllerHandler.cpp',
    'Handlers/DragRectHandler.cpp',
    'Handlers/InspectorHandler.cpp',
    'Handlers/LogHandler.cpp',
    'Handlers/MainHandler.cpp',
    'Handlers/ShortcutHandler.cpp',
    'Handlers/ViewHandler.cpp',

    'IO/Reader.cpp',
    'IO/STLReader.cpp',
    'IO/STLWriter.cpp',

    'Items/AppInfo.cpp',
    'Items/Board.cpp',
    'Items/BuildVolume.cpp',
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
    'Items/RadialMenuInfo.cpp',
    'Items/SessionState.cpp',
    'Items/Settings.cpp',
    'Items/Shelf.cpp',
    'Items/UnitConversion.cpp',

    'Managers/ActionManager.cpp',
    'Managers/AnimationManager.cpp',
    'Managers/BoardManager.cpp',
    'Managers/ClipboardManager.cpp',
    'Managers/CommandManager.cpp',
    'Managers/EventManager.cpp',
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
    'Math/BeveledMesh.cpp',
    'Math/Beveler.cpp',
    'Math/CGALInternal.cpp',
    'Math/ColorRing.cpp',
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
    'Math/Skeleton.cpp',
    'Math/TextUtils.cpp',
    'Math/ToString.cpp',
    'Math/Triangulation.cpp',
    'Math/Types.cpp',
    'Math/VertexRing.cpp',

    'Models/BeveledModel.cpp',
    'Models/BoxModel.cpp',
    'Models/CSGModel.cpp',
    'Models/ClippedModel.cpp',
    'Models/CombinedModel.cpp',
    'Models/ConvertedModel.cpp',
    'Models/CylinderModel.cpp',
    'Models/HullModel.cpp',
    'Models/ImportedModel.cpp',
    'Models/MirroredModel.cpp',
    'Models/Model.cpp',
    'Models/ParentModel.cpp',
    'Models/RevSurfModel.cpp',
    'Models/RootModel.cpp',
    'Models/SphereModel.cpp',
    'Models/TextModel.cpp',
    'Models/TorusModel.cpp',

    'Panels/ActionPanel.cpp',
    'Panels/BevelToolPanel.cpp',
    'Panels/CSGToolPanel.cpp',
    'Panels/DialogPanel.cpp',
    'Panels/FilePanel.cpp',
    'Panels/HelpPanel.cpp',
    'Panels/ImportToolPanel.cpp',
    'Panels/InfoPanel.cpp',
    'Panels/KeyboardPanel.cpp',
    'Panels/NameToolPanel.cpp',
    'Panels/Panel.cpp',
    'Panels/RadialMenuPanel.cpp',
    'Panels/RevSurfToolPanel.cpp',
    'Panels/SessionPanel.cpp',
    'Panels/SettingsPanel.cpp',
    'Panels/TextToolPanel.cpp',
    'Panels/TreePanel.cpp',

    'Panes/BoxPane.cpp',
    'Panes/ButtonPane.cpp',
    'Panes/CheckboxPane.cpp',
    'Panes/ClipPane.cpp',
    'Panes/ContainerPane.cpp',
    'Panes/DropdownPane.cpp',
    'Panes/GridPane.cpp',
    'Panes/IconPane.cpp',
    'Panes/ImagePane.cpp',
    'Panes/LabeledSliderPane.cpp',
    'Panes/Pane.cpp',
    'Panes/ProfilePane.cpp',
    'Panes/RadioButtonPane.cpp',
    'Panes/ScrollingPane.cpp',
    'Panes/SliderPane.cpp',
    'Panes/SpecialKeyPane.cpp',
    'Panes/SwitcherPane.cpp',
    'Panes/TextInputPane.cpp',
    'Panes/TextKeyPane.cpp',
    'Panes/TextPane.cpp',
    'Panes/TouchWrapperPane.cpp',

    'Parser/Field.cpp',
    'Parser/Object.cpp',
    'Parser/Parser.cpp',
    'Parser/Registry.cpp',
    'Parser/Scanner.cpp',
    'Parser/ValueWriter.cpp',
    'Parser/Writer.cpp',

    'SG/Box.cpp',
    'SG/ColorMap.cpp',
    'SG/Cylinder.cpp',
    'SG/Ellipsoid.cpp',
    'SG/FileImage.cpp',
    'SG/FileMap.cpp',
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
    'SG/MutableTriMeshShape.cpp',
    'SG/NamedColor.cpp',
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
    'SG/TriMeshShape.cpp',
    'SG/Uniform.cpp',
    'SG/UniformBlock.cpp',
    'SG/UniformDef.cpp',
    'SG/VRCamera.cpp',
    'SG/WindowCamera.cpp',

    'Targets/EdgeTarget.cpp',
    'Targets/PointTarget.cpp',

    'Tools/BevelTool.cpp',
    'Tools/CSGTool.cpp',
    'Tools/ClipTool.cpp',
    'Tools/ColorTool.cpp',
    'Tools/ComplexityTool.cpp',
    'Tools/CylinderTool.cpp',
    'Tools/FindTools.cpp',
    'Tools/ImportTool.cpp',
    'Tools/MirrorTool.cpp',
    'Tools/NameTool.cpp',
    'Tools/PanelTool.cpp',
    'Tools/PassiveTool.cpp',
    'Tools/RevSurfTool.cpp',
    'Tools/RotationTool.cpp',
    'Tools/ScaleTool.cpp',
    'Tools/TextTool.cpp',
    'Tools/Tool.cpp',
    'Tools/TorusTool.cpp',
    'Tools/TranslationTool.cpp',

    'Trackers/GripTracker.cpp',
    'Trackers/MouseTracker.cpp',
    'Trackers/PinchTracker.cpp',
    'Trackers/PointerTracker.cpp',
    'Trackers/TouchTracker.cpp',
    'Trackers/Tracker.cpp',

    'Util/Assert.cpp',
    'Util/Delay.cpp',
    'Util/FilePath.cpp',
    'Util/FilePathList.cpp',
    'Util/General.cpp',
    'Util/KLog.cpp',
    'Util/Read.cpp',
    'Util/StackTrace.cpp',
    'Util/String.cpp',
    'Util/URL.cpp',
    'Util/UTime.cpp',
    'Util/Write.cpp',

    'Viewers/GLFWViewer.cpp',
    'Viewers/VRViewer.cpp',

    'Widgets/DiscWidget.cpp',
    'Widgets/DraggableWidget.cpp',
    'Widgets/EdgeTargetWidget.cpp',
    'Widgets/GenericWidget.cpp',
    'Widgets/IconSwitcherWidget.cpp',
    'Widgets/IconWidget.cpp',
    'Widgets/PointTargetWidget.cpp',
    'Widgets/PushButtonWidget.cpp',
    'Widgets/RadialLayoutWidget.cpp',
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
    'Math/CGALInternal.cpp',
    'Math/MeshCombining.cpp',
    'Math/MeshValidation.cpp',
    'Math/Triangulation.cpp',
]

# Source files that needs special treatment on Windows because of size.
big_lib_sources = [
    'Math/MeshCombining.cpp',
]

# Source files that needs special treatment because of OpenVR
openvr_lib_sources = [
    'App/VRContext.cpp',
    'App/VRModelLoader.cpp',
]

# Add non-release sources.
if mode != 'rel':
    lib_sources += [
        'Debug/Dump3dv.cpp',
        'Debug/Print.cpp',
        'Debug/Shortcuts.cpp',
        'Panels/TestPanel.cpp',
    ]

# -----------------------------------------------------------------------------
# Platform-specific environment setup.
# -----------------------------------------------------------------------------

platform = str(Platform())
if platform.startswith('win'):
    platform = 'windows'
elif platform == 'darwin':
    platform = 'mac'
else:
    platform = 'linux'

if platform == 'windows':
    platform_env = Environment(tools = ['mingw'])
else:
    platform_env = Environment()

# OpenVR libraries have slightly different subdirectories.
if platform == 'windows':
    openvr_platform = 'win64'
elif platform == 'darwin':
    openvr_platform = 'osx32'  # Actually works for both 32- and 64-bit.
elif platform == 'linux':
    openvr_platform = 'linux64'

platform_env.Replace(
    PLATFORM        = platform,
    OPENVR_PLATFORM = openvr_platform,
    OPENVR_DIR      = '#submodules/openvr/bin/$OPENVR_PLATFORM',
    OPENVR_LIB      = '$OPENVR_DIR/${SHLIBPREFIX}openvr_api${SHLIBSUFFIX}',
)

# -----------------------------------------------------------------------------
# Base environment setup.
# -----------------------------------------------------------------------------

# Convenience to add double quotes to a CPPDEFINE.
def QuoteDef(s):
    return '"\\"' + s + '\\""'

# Send all build products to build_dir.
VariantDir(build_dir, 'src', duplicate=False)

base_env = platform_env.Clone()

# Set up directories for use below.
base_env.Replace(
    BUILD_DIR = build_dir,
    ION_DIR   = '#ionsrc/Ion',
)

base_env.Replace(
    CPPPATH = [
        '#/src',
        '$BUILD_DIR/include',  # Generated headers.
        '$ION_DIR',
        '$ION_DIR/ion/port/override',
        '$ION_DIR/third_party',         # For stblib.
        '$ION_DIR/third_party/google',
        '$ION_DIR/third_party/absl',
        '$ION_DIR/third_party/google',
        '$ION_DIR/third_party/image_compression',
        '#/submodules/docopt.cpp',
        '#/submodules/magic_enum/include',
        '#/submodules/openvr/headers',
    ],
    CPPDEFINES = [
        ('RESOURCE_DIR',  QuoteDef('./resources')),
        ('TEST_DATA_DIR', QuoteDef(Dir('#/src/Tests/Data').abspath)),

        # Required for Ion.
        'ARCH_K8',
        'OPENCTM_NO_CPP',
        ('ION_ARCH_X86_64', '1'),
        ('ION_NO_RTTI', '0'),
    ],
    LIBPATH = [
        '$BUILD_DIR',
        '$OPENVR_DIR',
    ],
    RPATH = [
        Dir(base_env.subst('$BUILD_DIR')).abspath,
        Dir(base_env.subst('$OPENVR_DIR')).abspath,
    ],
    LIBS = [
        'openvr_api',
        'ionshared',
        'mpfr', 'gmp',   # Required for CGAL.
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
        ],
        # Make sure the MSYS2 commands (such as cygpath) are available.
        ENV = {'PATH' : environ['PATH']},
        # This is needed for stack traces.
        LIBS = ['DbgHelp'],
    )
    # Note: the "-O1" keeps big files from choking on Windows ("string table
    # overflow", "file too big").
    big_cflags = ['-O1']
    run_program = f'c:/msys64/usr/bin/bash.exe bin/runwinprogram.bash {mode}'
    # Create the special version of openvr.h for Windows.
    base_env.Command(target='$BUILD_DIR/include/openvr.h',
                     source='#submodules/openvr/headers/openvr.h',
                     action='python bin/fix-openvr-header.py $SOURCES $TARGET')

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

if platform == 'windows':
    common_flags.append('-Wno-maybe-uninitialized')  # CGAL needs this.

openvr_cflags = ['-Wno-old-style-cast']  # openvr.h violates this a lot.

# -----------------------------------------------------------------------------
# Mode-specific environment setup.
# -----------------------------------------------------------------------------

# Specialize for debug, optimized, or release modes.
dbg_env = base_env.Clone(ENABLE_DEBUG_FEATURES = True)
opt_env = base_env.Clone(ENABLE_DEBUG_FEATURES = True)
rel_env = base_env.Clone(ENABLE_DEBUG_FEATURES = False)
dbg_env.Append(
    CXXFLAGS   = common_flags + ['-g'],
    LINKFLAGS  = common_flags + ['-g'],
    CPPDEFINES = [
        ('CHECK_GL_ERRORS',    'true'),
        ('DEBUG',              'true'),
        ('ENABLE_DASSERT',     'true'),
        ('ION_DEBUG',          'true'),
        '_DEBUG',
        # This allows valgrind to work on the debug executables.
        'CGAL_DISABLE_ROUNDING_MATH_CHECK',
        # ('ION_TRACK_SHAREABLE_REFERENCES', '1'),
    ],
)
opt_env.Append(
    CXXFLAGS   = common_flags + ['-O3'],
    LINKFLAGS  = common_flags + ['-O3', '-Wl,--strip-all'],
    CPPDEFINES = [
        ('CHECK_GL_ERRORS',    'false'),
    ],
)
rel_env.Append(
    CXXFLAGS   = common_flags + [
        '-O3',
        # Work around a GCC bug with some inlined code; see
        #  https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56456
        '-Wno-array-bounds',
    ],
    LINKFLAGS  = common_flags + ['-O3', '-Wl,--strip-all'],
    CPPDEFINES = [
        ('CHECK_GL_ERRORS',    'false'),
        ('RELEASE_BUILD',      'true'),
    ],
)
# The release executable always runs from its own directory.
rel_env.Replace(RPATH = ['.'])

mode_envs = {
    'opt' : opt_env,
    'dbg' : dbg_env,
    'rel' : rel_env
}
mode_env = mode_envs[mode]

mode_env.Append(
    CPPDEFINES = [('ENABLE_DEBUG_FEATURES',
                   1 if mode_env["ENABLE_DEBUG_FEATURES"] else 0)])

packages = [
    'freetype2',
    'glfw3',
    'jsoncpp',
    'libjpeg',
    'minizip',
    'stb',
    'tinyxml2',
    'zlib',
]

package_str = ' '.join(packages)
mode_env.ParseConfig(f'pkg-config {package_str} --cflags --libs')

# -----------------------------------------------------------------------------
# 'reg_env' is the regular environment, and 'cov_env' is the environment used
# to generate code coverage. Both are derived from mode_env.
# -----------------------------------------------------------------------------

reg_env = mode_env.Clone()
cov_env = mode_env.Clone()

cov_env.Append(
    CXXFLAGS  = ['--coverage' ],
    LINKFLAGS = ['--coverage' ],
)
# Use a different suffix for compiled sources so that they can be in the
# same directory as regular compiled sources.
cov_env.Replace(SHOBJSUFFIX = '_cov.os')

# Workaround for Windows: MSYS2 + SCons = some ridiculous length restriction
# for a command line. This causes library linking to fail.
# This fix is adapted from:
#   https://github.com/SCons/scons/wiki/LongCmdLinesOnWin32
if platform == 'windows':
    def winspawn(sh, escape, cmd, args, env):
        from subprocess import STARTUPINFO, STARTF_USESHOWWINDOW, Popen, PIPE
        newargs = ' '.join(args[1:])
        cmdline = cmd + " " + newargs
        startupinfo = STARTUPINFO()
        startupinfo.dwFlags |= STARTF_USESHOWWINDOW
        proc = Popen(cmdline, stdin=PIPE, stdout=PIPE,
                     stderr=PIPE, startupinfo=startupinfo,
                     shell=False, env=env, text=True)
        data, err = proc.communicate()
        rv = proc.wait()
        if rv:
            print(err)
        return rv
    for env in [reg_env, cov_env]:
        env['ORIG_SPAWN'] = env['SPAWN']  # Save for tests to use.
        env['SPAWN']      = winspawn

# -----------------------------------------------------------------------------
# Building main library so tests can link against it. It has to be a shared
# library so that it can link against other shared libraries.
# -----------------------------------------------------------------------------

def BuildObject(env, source):
    flags = []
    defs  = []
    if source in slow_lib_sources:
        flags += opt_env['CXXFLAGS']  # Always use optimized version
    else:
        flags += env['CXXFLAGS']
        if source in big_lib_sources:
            flags += big_cflags
        if source in openvr_lib_sources:
            flags += openvr_cflags
    # Special case for Tuning.cpp, which needs to have these defined.
    defs += env['CPPDEFINES']
    if source == 'Base/Tuning.cpp':
        defs += [
            ('APP_NAME',            QuoteDef(APP_NAME)),
            ('SESSION_SUFFIX',      QuoteDef(SESSION_SUFFIX)),
            ('VERSION_STRING',      QuoteDef(VERSION_STRING)),
            ('PUBLIC_DOC_BASE_URL', QuoteDef(PUBLIC_DOC_BASE_URL)),
        ]

    return env.SharedObject(source=f'$BUILD_DIR/{source}',
                            CXXFLAGS=flags, CPPDEFINES=defs)

main_lib = APP_NAME

# Build regular and coverage-enabled object files.
reg_lib_objects = [BuildObject(reg_env, source) for source in lib_sources]
cov_lib_objects = [BuildObject(cov_env, source) for source in lib_sources]

reg_lib = reg_env.SharedLibrary(f'$BUILD_DIR/{main_lib}',     reg_lib_objects)
cov_lib = cov_env.SharedLibrary(f'$BUILD_DIR/{main_lib}_cov', cov_lib_objects)

reg_env.Alias('Libs', reg_lib)

# -----------------------------------------------------------------------------
# Building main and related applications. No need for a coverage-enabled
# version.
# -----------------------------------------------------------------------------

main_app_name = APP_NAME

# Build the applications.
apps = [main_app_name, 'nodeviewer', 'printtypes']
if platform == 'linux':
    apps += ['createcheatsheet', 'snapimage']

apps_extra_sources = [
    '$BUILD_DIR/App/Args.cpp',
    '$BUILD_DIR/App/SnapScript.cpp',
]

app_env = reg_env.Clone()
app_env.Append(
    LIBPATH = ['$BUILD_DIR/docopt.cpp'],
    LIBS    = [main_lib, 'docopt'],
)

# Avoid opening a cmd window with the application on Windows (Release only -
# otherwise no logging output can appear!)
if platform == 'windows' and mode == "rel":
    app_env.Append(LINKFLAGS = '-Wl,-subsystem,windows')

main_app  = None
snapimage = None
for app_name in apps:
    app = app_env.Program(
        f'$BUILD_DIR/Apps/{app_name}',
        [f'$BUILD_DIR/Apps/{app_name}.cpp'] + apps_extra_sources)
    app_env.Default(app)
    app_env.Alias('Apps', app)

    # Main app is special
    if app_name == main_app_name:
        main_app = app[0]
    elif app_name == 'snapimage':
        snapimage = app[0]

# -----------------------------------------------------------------------------
# Include Ion, submodule, resources, test, internal doc, and public doc build
# files.
# -----------------------------------------------------------------------------

exports = {
    'submodules'   : ['brief', 'build_dir', 'platform_env'],
    'tests'        : ['reg_env', 'cov_env', 'main_lib', 'run_program',
                      'cov_lib_objects'],
    'internal_doc' : ['doc_build_dir', 'APP_NAME', 'VERSION_STRING'],
    'public_doc'   : ['doc_build_dir', 'snapimage',
                      'APP_NAME', 'SESSION_SUFFIX', 'VERSION_STRING'],
    'ion_lib'      : ['brief', 'build_dir', 'mode', 'platform_env'],
}

# Build Ion on all platforms.
ion_lib = SConscript('ionsrc/Ion/SConscript',  exports['ion_lib'],
                     variant_dir=f'{build_dir}/Ion', duplicate=False)

# Build submodules and tests on all platforms.
SConscript('submodules/SConscript', exports['submodules'])
SConscript('src/Tests/SConscript',  exports['tests'],
           variant_dir=f'{build_dir}/Tests')

# Documentation and menu icons are built only on Linux. Building on different
# platforms creates slightly different image files, causing git thrashing. No
# need for that once everything is built.
if platform == 'linux':
    reg_env.Alias('Icons', SConscript('resources/SConscript'))
    # Applications depend on the icons.
    reg_env.Depends('Apps', 'Icons')

    internal_doc = SConscript('InternalDoc/SConscript', exports['internal_doc'])
    public_doc   = SConscript('PublicDoc/SConscript',   exports['public_doc'])
    reg_env.Alias('InternalDoc', [internal_doc])
    reg_env.Alias('PublicDoc',   [public_doc])

# -----------------------------------------------------------------------------
# Building the release as a Zip file.
# -----------------------------------------------------------------------------

# NOTE: The 'Zip()' builder in SCons claims to allow the command to be modified
# with the 'ZIP', 'ZIPFLAGS', and other construction variables. However, they
# are actually not used in the SCons.Tool.zip source file. So do everything
# manually here.

def BuildZipFile(target, source, env):
    from os      import walk
    from os.path import basename, dirname, isfile, join, relpath
    from zipfile import ZipFile
    zf = ZipFile(str(target[0]), 'w')
    def AddFile(f, rel_path):
        zf.write(f, f'{APP_NAME}/{rel_path}')
    for src in source:
        name = str(src)
        # Walk through directories.
        if src.isdir():
            basedir = dirname(name)
            for root, dirs, files in walk(name):
                for f in files:
                    path = join(root, f)
                    if isfile(path):
                        AddFile(path, relpath(path, basedir))
        else:  # Regular file.
            AddFile(name, basename(name))

# Zip the executable, all 3 shared libraries, and the resources dir.
zip_input  = [main_app, reg_lib, ion_lib, '$OPENVR_LIB', 'resources']
zip_name   = f'{APP_NAME}-{VERSION_STRING}-{platform.capitalize()}'
zip_output = f'$BUILD_DIR/Release/{zip_name}.zip'

# Windows requires all dependent libraries to be present.
zip_win_mingw_libs = [
    'glfw3',
    'libbrotlicommon',
    'libbrotlidec',
    'libbz2-1',
    'libfreetype-6',
    'libgcc_s_seh-1',
    'libglib-2.0-0',
    'libgmp-10',
    'libgraphite2',
    'libharfbuzz-0',
    'libiconv-2',
    'libintl-8',
    'libpcre-1',
    'libpng16-16',
    'libstdc++-6',
    'libwinpthread-1',
    'zlib1',
]
if platform == 'windows':
    from os import popen
    run_cygpath = f'c:/msys64/usr/bin/bash.exe -c "cygpath -m /mingw64/bin"'
    mingw_dir = popen(run_cygpath).read().replace('\n', '')
    zip_input += [f'{mingw_dir}/{lib}.dll' for lib in zip_win_mingw_libs]

rel = rel_env.Command(zip_output, zip_input, BuildZipFile)
rel_env.Alias('Release', rel)

# -----------------------------------------------------------------------------
# Other Aliases.
# -----------------------------------------------------------------------------

reg_env.Alias('Ion', ion_lib)

