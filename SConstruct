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

# -----------------------------------------------------------------------------
# Environment setup.
# -----------------------------------------------------------------------------

# Create all SCons environments.
envs = SConscript('SConscript_env', exports=['mode'])

base_env = envs['base']
base_env.SConsignFile('$BUILD_DIR/sconsign.dblite')  # For easy cleanup.

# Shorten compile/link lines for clarity.
if brief:
    for env in envs.values():
        Brief(env)

# Set this for easy access below.
build_dir = base_env.subst('$BUILD_DIR')

# -----------------------------------------------------------------------------
# Building.
# -----------------------------------------------------------------------------

# Variables that are exported to various SConscript files.
exports = {
    'submodules'   : ['brief', 'build_dir', 'platform_env'],
    'tests'        : ['reg_env', 'cov_env', 'main_lib', 'run_program',
                      'cov_lib_objects'],
    'internal_doc' : ['doc_build_dir', 'APP_NAME', 'VERSION_STRING'],
    'public_doc'   : ['doc_build_dir', 'snapimage',
                      'APP_NAME', 'SESSION_SUFFIX', 'VERSION_STRING'],
}

# Build the Ion library.
ion_env = envs['ion']
ion_lib = SConscript('ionsrc/SConscript_ion', exports=['ion_env'],
                     variant_dir=f'{build_dir}/Ion', duplicate=False)

# Build the application library and applications.
lib_env = envs['lib']
app_env = envs['app']
(app_lib, apps) = SConscript('src/SConscript_src',
                             exports=['app_env', 'app_dict',
                                      'ion_lib', 'lib_env'],
                             variant_dir=build_dir, duplicate=False)
