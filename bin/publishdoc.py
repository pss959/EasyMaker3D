#!/usr/bin/env python3

# -----------------------------------------------------------------------------
# See main() for description and usage.
# -----------------------------------------------------------------------------

from argparse   import ArgumentParser
from os         import makedirs
from os.path    import dirname, join, realpath
from re         import search
from subprocess import Popen, PIPE, run
from sys        import argv, exit

do_dry_run  = False
do_commit   = True
root_dir    = ''

#-----------------------------------------------------------------------------
# Command-line argument processing.
#-----------------------------------------------------------------------------

def ProcessArguments():
    description = (
        """Publishes the current public documentation to Github Pages in the
        master  branch under the "docs/<version-string>/" subdirectory.""")
    parser = ArgumentParser(description=description)
    parser.add_argument(
        '-n', '--dry_run', action='store_true', dest='dry_run', default=False,
        help='Do a dry run, just printing out what would happen.')
    parser.add_argument(
        '-c', '--no_commit', action='store_true', dest='no_commit',
        default=False, help='Skip git commit (and push steps).')
    return parser

# -----------------------------------------------------------------------------
# Runs a command if the dry_run argument is not set.
# -----------------------------------------------------------------------------

def Run(cmd_str):
    print(f'---   Running: "{cmd_str}"')
    if not do_dry_run:
        run(cmd_str.split(), cwd=root_dir)

#-----------------------------------------------------------------------------
# Mainline.
#-----------------------------------------------------------------------------

def main():
    global do_dry_run
    global do_commit
    global root_dir

    parser  = ProcessArguments()
    args    = parser.parse_args()
    do_dry_run = args.dry_run
    do_commit  = not args.no_commit

    # Get the root directory as an absolute path.
    root_dir = dirname(dirname(realpath(argv[0])))

    # Get the current version string.
    version_line = Popen(['grep', 'version_string =', f'{root_dir}/SConstruct'],
                         stdout=PIPE, text=True).stdout.read()
    version = search(r"'([^']+)'", version_line).group(1)

    # Make sure the master branch is current.
    print('--- Switching to the master branch.')
    Run('git checkout -q master')

    # Make sure nothing has changed in the master branch if committing.
    if (do_commit and Popen(['git', 'status', '--porcelain'],
                            stdout=PIPE, text=True).stdout.read()):
        print('*** The master branch has uncommitted changes; exiting.')
        if do_dry_run:
            print('*** Remaining dry-run commands to follow:')
        else:
            exit(1)

    # Make sure the doc is up to date.
    print(f'--- Building the doc for version {version}.')
    Run('scons PublicDoc')

    # Sync all generated doc into the 'docs' directory.
    print(f'--- Syncing the HTML doc into the docs directory.')
    makedirs('docs', exist_ok=True)
    Run(f'rsync -vau --delete --exclude=.nojekyll build/PublicDoc/ docs/')

    # Commit the results.
    if do_commit:
        print('--- Committing to the master branch.')
        Run('git add docs/')
        Run(f'git commit -m "Updating public docs for version {version}"'
            ' -- docs/')

        # Push to local repo and Github.
        print('--- Pushing to local repo and Github.')
        Run('git push origin master')
        Run('git push github master')

if __name__ == '__main__':
    main()
