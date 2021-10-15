# -----------------------------------------------------------------------------
# Configuration.
# -----------------------------------------------------------------------------

import itertools

AddOption('--mode',
          dest='mode',
          type='string',
          nargs=1,
          action='store',
          default='dbg',
          help='build mode ("dbg" or "opt")')

AddOption('--brief',
          dest='brief',
          action='store_true',
          default=False,
          help='make output brief')

# All build products go into this directory.
build_dir = f'build/{GetOption("mode")}'

# -----------------------------------------------------------------------------
# Environment setup.
# -----------------------------------------------------------------------------

# Send all build products to build_dir.
VariantDir(build_dir, '.', duplicate = 0)

common_flags = [
    '--std=c++14',
    '-Wall',
    '-Wno-strict-aliasing',  # Ion has issues with this.
]

env = Environment(
    CPPPATH   = [
        '/usr/include',
        '/usr/include/jsoncpp',  # Use JSONCPP from system.
        '#',
        '#/ion/port/override',
        '#third_party/absl',
        '#third_party/google',
        '#third_party/image_compression',
    ],
    CPPDEFINES = [
        'ARCH_K8',
        'OPENCTM_NO_CPP',
        ('ION_API', ''),
        ('ION_APIENTRY', ''),
        ('ION_ARCH_X86_64', '1'),
        ('ION_NO_RTTI', '0'),
        ('ION_PLATFORM_LINUX', '1'),
        ('JSON_INC', 'jsoncpp/json'),
    ],
    LIBPATH   = [],
    CXXFLAGS  = common_flags,
    LINKFLAGS = common_flags + [
        '-Wl,--unresolved-symbols=report-all',
    ],
    LIBS = [
        'dl',
        'pthread',
    ],
)

packages = [
    'freetype2',
    'jsoncpp',
    'libjpeg',
    'minizip',
    'stb',
    'tinyxml2',
    'zlib',
]

env.ParseConfig(f'pkg-config {" ".join(packages)} --cflags --libs')

# Create SCons's database file in the build directory for easy cleanup.
env.SConsignFile(f'{build_dir}/sconsign.dblite')

# Specialize for debug or optimized modes.
if GetOption('mode') == 'opt':
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
            '_DEBUG',
            ('DEBUG', '1'),
            ('ION_DEBUG', '1'),
        ],
    )

# Handle brief mode.
if GetOption('brief'):
    env.Replace(
        CCCOMSTR    = "Compiling C $TARGET",
        CXXCOMSTR   = "Compiling C++ $TARGET",
        LINKCOMSTR  = "Linking   $TARGET",
        SHCCCOMSTR  = "Compiling C $TARGET",
        SHCXXCOMSTR = "Compiling C++ $TARGET",
    )

# Creating source files from asset definition files.
zipasset_builder = Builder(
    action = './ion/dev/zipasset_generator.py $SOURCE . $TARGET',
    suffix='.cc', src_suffix='.iad')
env.Append(BUILDERS = { 'ZipAsset' : zipasset_builder })

# -----------------------------------------------------------------------------
# Source files. A "name" means there is both a .h header and a .cc source file.
# -----------------------------------------------------------------------------

def MakeTuple(subdir, names=[], headers=[], sources=[], assets=[]):
    return (subdir, names, headers, sources, assets)

# Each entry is a tuple: (subdir, names, headers, sources).
ion_files = [
    MakeTuple(subdir='analytics',
              names=[
                  'benchmark',
                  'benchmarkutils',
                  'gpuperformance',
              ],
              headers=['discrepancy.h']),
    MakeTuple(subdir='base',
              names=[
                  'allocatable',
                  'allocationmanager',
                  'allocator',
                  'bufferbuilder',
                  'calllist',
                  'datacontainer',
                  'datetime',
                  'fullallocationtracker',
                  'invalid',
                  'logchecker',
                  'logging',
                  'memoryzipstream',
                  'notifier',
                  'readwritelock',
                  'setting',
                  'settingmanager',
                  'spinmutex',
                  'staticsafedeclare',
                  'stringtable',
                  'stringutils',
                  'threadspawner',
                  'utf8iterator',
                  'workerpool',
                  'zipassetmanager',
              ],
              headers=[
                  'allocationsizetracker.h',
                  'allocationtracker.h',
                  'argcount.h',
                  'array2.h',
                  'circularbuffer.h',
                  'enumhelper.h',
                  'functioncall.h',
                  'indexmap.h',
                  'lockguards.h',
                  'nulllogentrywriter.h',
                  'once.h',
                  'referent.h',
                  'scalarsequence.h',
                  'scopedallocation.h',
                  'serialize.h',
                  'shareable.h',
                  'sharedptr.h',
                  'signal.h',
                  'static_assert.h',
                  'threadlocalobject.h',
                  'type_structs.h',
                  'variant.h',
                  'varianttyperesolver.h',
                  'vectordatacontainer.h',
                  'weakreferent.h',
                  'zipassetmanagermacros.h',
              ]),
    MakeTuple(subdir='gfx',
              names=[
                  'attribute',
                  'attributearray',
                  'bufferobject',
                  'computeprogram',
                  'cubemaptexture',
                  'framebufferobject',
                  'graphicsmanager',
                  'image',
                  'indexbuffer',
                  'node',
                  'renderer',
                  'resourceholder',
                  'resourcemanager',
                  'sampler',
                  'shader',
                  'shaderinput',
                  'shaderinputregistry',
                  'shaderprogram',
                  'shape',
                  'statetable',
                  'texture',
                  'tracecallextractor',
                  'tracinghelper',
                  'tracingstream',
                  'uniform',
                  'uniformblock',
                  'uniformholder',
                  'updatestatetable',
              ],
              headers=[
                  'glfunctiontypes.h',
                  'graphicsmanagermacrodefs.h',
                  'graphicsmanagermacroundefs.h',
                  'openglobjects.h',
                  'transformfeedback.h',
              ],
              sources=['tracinghelperenums.cc']),
    MakeTuple(subdir='gfxprofile',
              names=['gpuprofiler']),
    MakeTuple(subdir='gfxutils',
              names=[
                  'buffertoattributebinder',
                  'frame',
                  'printer',
                  'shadermanager',
                  'shadersourcecomposer',
                  'shapeutils',
              ],
              headers=['resourcecallback.h']),
    MakeTuple(subdir='image',
              names=[
                  'conversionutils',
                  'exportjpeg',
                  'ninepatch',
                  'renderutils',
              ]),
    MakeTuple(subdir='math',
              names=[
                  'matrixutils',
                  'rotation',
                  'transformutils',
              ],
              headers=[
                  'angle.h',
                  'angleutils.h',
                  'fieldofview.h',
                  'matrix.h',
                  'range.h',
                  'rangeutils.h',
                  'utils.h',
                  'vector.h',
                  'vectorutils.h',
              ]),
    MakeTuple(subdir='port',
              names=[
                  'barrier',
                  'break',
                  'environment',
                  'fileutils',
                  'logging',
                  'memory',
                  'memorymappedfile',
                  'semaphore',
                  'stacktrace',
                  'string',
                  'threadutils',
                  'timer',
              ],
              headers=[
                  'align.h',
                  'atomic.h',
                  'macros.h',
                  'nullptr.h',
                  'static_assert.h',
                  'trace.h',
                  'useresult.h',
              ],
              sources=[
                  'logging_cerr.cc',
              ]),
    MakeTuple(subdir='portgfx',
              names=[
                  'glcontext',
                  'isextensionsupported',
                  'setswapinterval',
              ],
              headers=[
                  'glenums.h',
                  'glheaders.h',
              ],
              sources=[
                  'glxcontext.cc',
              ]),
    MakeTuple(subdir='profile',
              names=[
                  'calltracemanager',
                  'profiling',
                  'timeline',
                  'timelineevent',
                  'timelinenode',
                  'tracerecorder',
                  'vsyncprofiler',
              ],
              headers=[
                  'timelineframe.h',
                  'timelinemetric.h',
                  'timelinerange.h',
                  'timelinescope.h',
                  'timelinesearch.h',
                  'timelinethread.h',
              ]),
    MakeTuple(subdir='remote',
              names=[
                  'calltracehandler',
                  'httpserver',
                  'nodegraphhandler',
                  'remoteserver',
                  'resourcehandler',
                  'settinghandler',
                  'shaderhandler',
                  'tracinghandler',
              ],
              assets=[
                  'res/calltrace.iad',
                  'res/geturi_cc.iad',
                  'res/nodegraph.iad',
                  'res/resources.iad',
                  'res/root.iad',
                  'res/settings.iad',
                  'res/shader_editor.iad',
                  'res/tracing.iad',
              ]),
    MakeTuple(subdir='text',
              names=[
                  'basicbuilder',
                  'binpacker',
                  'builder',
                  'font',
                  'fontimage',
                  'fontmanager',
                  'freetypefont',
                  'freetypefontutils',
                  'icuutils',
                  'layout',
                  'outlinebuilder',
                  'sdfutils',
              ],
              headers=[
                  'fontmacros.h',
              ]),
]

# Special case for third_party code that is built inside Ion.
third_party_env = env.Clone(
    CPPDEFINES = [('IS_LITTLE_ENDIAN', '1')],
    CPPPATH = [
        '#',
        '#third_party/absl',
        '#third_party/google',
        '#third_party/image_compression',
        '#third_party/openctm/tools/rply',
    ],
)
third_party_sources = [
    'lodepng/lodepng.cpp',
    'mongoose/mongoose.c',
    'omaha/omaha/base/security/b64.c',
    'openctm/tools/3ds.cpp',
    'openctm/tools/common.cpp',
    'openctm/tools/dae.cpp',
    'openctm/tools/lwo.cpp',
    'openctm/tools/mesh.cpp',
    'openctm/tools/obj.cpp',
    'openctm/tools/off.cpp',
    'openctm/tools/ply.cpp',
    'openctm/tools/rply/rply.c',
    'openctm/tools/stl.cpp',
    'unzip/unzip.c',
    'image_compression/image_compression/internal/compressor4x4_helper.cc',
    'image_compression/image_compression/internal/dxtc_compressor.cc',
    'image_compression/image_compression/internal/dxtc_const_color_table.cc',
    'image_compression/image_compression/internal/dxtc_to_etc_transcoder.cc',
    'image_compression/image_compression/internal/etc_compressor.cc',
    'image_compression/image_compression/internal/pixel4x4.cc',
    'image_compression/image_compression/internal/pvrtc_compressor.cc',
]

third_party_objects = [
    third_party_env.SharedObject(f'{build_dir}/third_party/{source}')
    for source in third_party_sources]

# -----------------------------------------------------------------------------
# Building targets.
# -----------------------------------------------------------------------------

def AssetToSource(asset):
    return asset.replace('.iad', '.cc')

# Concatenate all source files.
ion_sources = []
for (subdir, names, headers, sources, assets) in ion_files:
    ion_sources += [f'{build_dir}/ion/{subdir}/{name}.cc' for name in names]
    ion_sources += [f'{build_dir}/ion/{subdir}/{source}' for source in sources]
    for asset in assets:
        ion_sources.append(env.ZipAsset(f'{build_dir}/ion/{subdir}/{asset}'))

env.SharedLibrary(f'{build_dir}/ionshared',
                  ion_sources + third_party_objects)
