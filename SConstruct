from brief import Brief
from os    import environ

# -----------------------------------------------------------------------------
# Configuration.
# -----------------------------------------------------------------------------

app_dict = {
    'APP_NAME'            : 'MakerVR',
    'SESSION_SUFFIX'      : '.mvr',
    'VERSION_STRING'      : '0.0.1',
    'PUBLIC_DOC_BASE_URL' : 'https://pss959.github.io/MakerVR-dist/',
}
doc_build_dir = 'build'

AddOption('--mode', dest='mode', type='string', nargs=1, action='store',
          default='opt', metavar='dbg|opt|rel|cov',
          help='optimized/debug/release/coverage mode')
AddOption('--brief', dest='brief', action='store_true',
          default='True', help='Shortened vs. full output')
AddOption('--nobrief', dest='brief', action='store_false',
          default='True', help='Shortened vs. full output')

mode  = GetOption('mode')
brief = GetOption('brief')

# -----------------------------------------------------------------------------
# Environment setup.
# -----------------------------------------------------------------------------

# Create all SCons environments. This returns a dictionary of environments.
envs = SConscript('SConscript_env', exports=['mode'])

base_env = envs['base']
base_env.SConsignFile('$BUILD_DIR/sconsign.dblite')  # For easy cleanup.

# Shorten compile/link lines for clarity.
if brief:
    for env in envs.values():
        Brief(env)

# Set these for easy access below.
build_dir = base_env.subst('$BUILD_DIR')
platform  = base_env['PLATFORM']

# -----------------------------------------------------------------------------
# Building.
# -----------------------------------------------------------------------------

# Build the Ion library.
ion_env = envs['ion']
ion_lib = SConscript('ionsrc/SConscript_ion', exports=['ion_env'],
                     variant_dir=f'{build_dir}/ionsrc', duplicate=False)

# Build submodules.
SConscript('submodules/SConscript_submodules', exports=['base_env'])

# Build the application library.
lib_env = envs['lib']
lib_env.Append(LIBS = [ion_lib])
app_lib = SConscript('src/SConscript_src', exports=['app_dict', 'lib_env'],
                     variant_dir=build_dir, duplicate=False)

# Build the applications. 'apps' is a dictionary mapping app name to executable.
app_env = envs['app']
app_env.Append(LIBS = ['AppLib', 'ionshared'])
apps = SConscript('src/Apps/SConscript_apps', exports=['app_dict', 'app_env'],
                  variant_dir=f'{build_dir}/Apps', duplicate=False)
app_env.Alias('Apps', apps.values())

# Build tests.
test_env = envs['test']
test_env.Append(LIBS = [app_lib])
SConscript('src/Tests/SConscript_tests', exports=['test_env'],
           variant_dir=f'{build_dir}/Tests')

# Build documentation and menu icons only on Linux. (Building on different
# platforms creates slightly different image files, causing git thrashing. No
# need for that once everything is built once.)
if platform == 'linux':
    # Applications depend on the icons.
    app_env.Alias('Icons', SConscript('resources/SConscript_resources'))
    app_env.Depends('Apps', 'Icons')

    # Internal (Doxygen-generated) web documentation.
    base_env.Alias('InternalDoc',
                   SConscript('InternalDoc/SConscript_dox',
                              exports=['app_dict', 'doc_build_dir']))
    # Public (Sphinx-generated) web documentation. This requires the
    # 'snapimage' application.
    snapimage = apps['snapimage'][0]
    base_env.Alias('PublicDoc',
                   SConscript('PublicDoc/SConscript_doc',
                              exports=['app_dict', 'doc_build_dir',
                                       'snapimage']))

# -----------------------------------------------------------------------------
# Building the release as a Zip file.
# -----------------------------------------------------------------------------

# NOTE: The 'Zip()' builder in SCons claims to allow the command to be modified
# with the 'ZIP', 'ZIPFLAGS', and other construction variables. However, they
# are actually not used in the SCons.Tool.zip source file. So do everything
# manually here.
def BuildZipFile_(target, source, env):
    from os      import walk
    from os.path import basename, dirname, isfile, join, relpath
    from zipfile import ZipFile
    zf = ZipFile(str(target[0]), 'w')
    def AddFile(f, rel_path):
        zf.write(f, f'{app_dict["APP_NAME"]}/{rel_path}')
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

if mode == 'rel':
    # Zip the executable, all 3 shared libraries, and the resources dir.
    main_app   = apps[app_dict['APP_NAME']]
    zip_input  = [main_app, app_lib, ion_lib, '$OPENVR_LIB', 'resources']
    zip_name   = (f'{app_dict["APP_NAME"]}-{app_dict["VERSION_STRING"]}-' +
                  f'{platform.capitalize()}')
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

    rel = app_env.Command(zip_output, zip_input, BuildZipFile_)
    app_env.Alias('Release', rel)
