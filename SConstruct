# -----------------------------------------------------------------------------
# Configuration.
# -----------------------------------------------------------------------------

# This version string has to be in exactly this format for the publishdoc script.
version_string = '1.7.0'

app_name = 'EasyMaker3D'

app_dict = {
    'APP_NAME'            : app_name,
    'COPYRIGHT'           : '2021-2023 Paul S. Strauss',
    'DATA_EXTENSION'      : '.emd',
    'SESSION_EXTENSION'   : '.ems',
    'VERSION_STRING'      : version_string,
    'PUBLIC_DOC_BASE_URL' : f'https://pss959.github.io/{app_name}',
    'GITHUB_URL'          : f'https://github.com/pss959/{app_name}',
}
doc_build_dir = 'build'

AddOption('--mode', dest='mode', type='string', nargs=1, action='store',
          default='opt', metavar='dbg|opt|rel|cov',
          help='optimized/debug/release/coverage mode')
AddOption('--brief', dest='brief', action='store_true',
          default='True', help='Shortened vs. full output')
AddOption('--nobrief', dest='brief', action='store_false',
          default='True', help='Shortened vs. full output')
AddOption('--noimages', dest='noimages', action='store_true',
          default='False', help='Do not update images for PublicDoc')

mode     = GetOption('mode')
brief    = GetOption('brief')
noimages = GetOption('noimages')

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
ion_env      = envs['ion']
ion_test_env = envs['iontest']
ion_lib = SConscript('ionsrc/SConscript_ion',
                     exports=['ion_env', 'ion_test_env'],
                     variant_dir=f'{build_dir}/ionsrc', duplicate=False)

# Build submodules.
SConscript('submodules/SConscript_submodules', exports=['base_env'])

# Build the application library.
lib_env = envs['lib']
app_lib = SConscript('src/SConscript_src', exports=['app_dict', 'lib_env'],
                     variant_dir=build_dir, duplicate=False)

# Build the applications. 'apps' is a dictionary mapping app name to executable.
app_env = envs['app']
app_env.Append(LIBS = ['$APP_LIB_NAME'])
apps = SConscript('src/Apps/SConscript_apps', exports=['app_dict', 'app_env'],
                  variant_dir=f'{build_dir}/Apps', duplicate=False)
app_env.Alias('Apps', apps.values())
app_env.Alias('App',  apps[app_name])

# Build tests.
test_env = envs['test']
test_env.Append(LIBS = [app_lib])
SConscript('src/Tests/SConscript_tests', exports=['ion_env', 'test_env'],
           variant_dir=f'{build_dir}/Tests', duplicate=False)

# Build documentation and menu icons only on Linux. (Building on different
# platforms creates slightly different image files, causing git thrashing. No
# need for that once everything is built once.)
if platform == 'linux':
    # Applications depend on the icons.
    app_env.Alias('Icons', SConscript('resources/SConscript_resources'))
    for app in apps.values():
        app_env.Depends(app, 'Icons')

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
                                       'noimages', 'snapimage']))

# -----------------------------------------------------------------------------
# Building the release as a Zip file.
# -----------------------------------------------------------------------------

if mode == 'rel':
    version  = app_dict['VERSION_STRING']
    rel_name = (f'{app_name}-{version}-{platform.capitalize()}')
    app      = apps[app_name]

    # Make a list of items needed to build the release. These are passed as a
    # dictionary so that they can be modified if necessary (see Mac).
    rel_dict = {
        'app'      : apps[app_name],
        'app_env'  : app_env,
        'app_lib'  : app_lib,
        'app_name' : app_name,
        'ion_lib'  : ion_lib,
        'ovr_lib'  : '$OPENVR_LIB',
        'rel_name' : rel_name,
        'res_dir'  : 'resources',
    }
    rel = SConscript('SConscript_release', exports=['rel_dict'])
    app_env.Alias('Release', rel)

# -----------------------------------------------------------------------------
# Creating or updating TAGS file for emacs.
# -----------------------------------------------------------------------------

tags = base_env.Command('TAGS', [], 'etags -R ionsrc/ion/ src/')
base_env.AlwaysBuild(tags)
