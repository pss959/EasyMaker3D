#!/usr/bin/env python3

# -----------------------------------------------------------------------------
# Parses "#include" statements from all files under the src/ directory to
# discover inter-module dependencies. Outputs a DOT graph to standard output.
#
# Running the "tred" command on the output will simplify the graph a lot to
# produce a better layer diagram.
# -----------------------------------------------------------------------------

from os         import chdir
from os.path    import dirname, join, realpath
from re         import match
from subprocess import Popen, PIPE, run

# Finds all module dependencies from include lines. Returns a dictionary
# mapping each module to all dependent modules.
def FindDependencies_():
    lines = Popen(['grep', '-r', '#include "', '.'],
                  stdout=PIPE, text=True).stdout.read().splitlines()
    dep_dict = {}
    for line in lines:
        if 'Tests/' in line:
            continue
        result = match(r'\./([^/]+)/.*:#include "([^/]+)', line)
        if result:
            from_mod = result.group(1)
            to_mod   = result.group(2)
            if from_mod != to_mod:
                if not from_mod in dep_dict:
                    dep_dict[from_mod] = set([to_mod])
                else:
                    dep_dict[from_mod].add(to_mod)
    return dep_dict

def WriteDot_(dep_dict):
    print('strict digraph {')
    for k, v in sorted(dep_dict.items()):
        deps = ' '.join(sorted(v))
        print(f'  {k};')
        print(f'  {k} -> {{ {deps} }};')
    print('}')

def main():
    script_dir = dirname(realpath(__file__))
    src_dir = join(script_dir, '../src')
    chdir(src_dir)
    dep_dict = FindDependencies_()
    WriteDot_(dep_dict)

if __name__ == '__main__':
    main()
