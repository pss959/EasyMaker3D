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
if brief:
    for env in envs.values():
        env.MakeBrief()

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

if mode == 'rel':
    app_name = app_dict['APP_NAME']
    version  = app_dict['VERSION_STRING']
    rel_name = (f'{app_name}-{version}-{platform.capitalize()}')

    # Make a list of main files to include in the release.
    rel_files = [apps[app_name], app_lib, ion_lib, '$OPENVR_LIB', 'resources']
    rel = SConscript('SConscript_release',
                     exports=['app_env', 'app_name', 'rel_files', 'rel_name'])
    app_env.Alias('Release', rel)
