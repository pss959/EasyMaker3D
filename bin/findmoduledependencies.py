#!/usr/bin/env python3

#@@@@
# SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
# SPDX-License-Identifier: AGPL-3.0-or-later
#@@@@

# -----------------------------------------------------------------------------
# Parses "#include" statements from all files under the src/ directory to
# discover intra- and inter-module dependencies. Outputs a DOT graph to
# standard output.
#
# Running the "tred" command on the output will simplify the graph to produce a
# better layer diagram.
# -----------------------------------------------------------------------------

from os         import chdir
from os.path    import dirname, join, realpath
from re         import match
from subprocess import Popen, PIPE

groups = [s.split() for s in [
    'Apps',
    'App Executors Managers Tools VR Viewers',
    'Agents Commands Handlers Trackers Panels Panes Selection',
    'Feedback IO Items Models Place Widgets',
    'Base Enums Math Parser SG Util']]

# Set up module-to-group dictionary (using group index).
mod_to_group = {}
for i, mods in enumerate(groups):
    for mod in mods:
        mod_to_group[mod] = i

# Adds to a dependency dictionary, avoiding self-dependencies.
def AddToDict_(dep_dict, from_dep, to_dep):
    if from_dep != to_dep:
        if not from_dep in dep_dict:
            dep_dict[from_dep] = set([to_dep])
        else:
            dep_dict[from_dep].add(to_dep)

# Finds all module-to-module dependencies from include lines. Returns a
# dictionary mapping each module to a set of all dependent modules.
def FindModToModDependencies_():
    lines = Popen(['grep', '-r', '#include "', '.'],
                  stdout=PIPE, text=True).stdout.read().splitlines()
    mod_to_mod = {}
    for line in lines:
        if not 'Tests/' in line and not 'Debug/' in line:
            result = match(r'\./([^/]+)/.*:#include "([^/]+)', line)
            if result:
                AddToDict_(mod_to_mod, result.group(1), result.group(2))
    return mod_to_mod

# For the given group, returns a dictionary with all intra-group
# module-to-module dependencies.
def FindIntraGroupDependencies_(group_dict, module_deps, group):
    intra_group = {}
    for k, v in module_deps.items():
        if group_dict[k] == group:
            for dep in v:
                if group_dict[dep] == group:
                    AddToDict_(intra_deps, k, dep)
    return intra_deps

# Given the dependency dictionary and individual group dependency
# dictionaries, returns a dictionary containing inter-group dependencies.
def FindInterGroupDependencies_(group_dict, module_deps, intra_deps):
    inter_deps = {}
    for k, v in module_deps.items():
        fc = group_dict[k]
        for dep in v:
            tc = group_dict[dep]
            if fc != tc:
                AddToDict_(inter_deps, fc, tc)
    return inter_deps

def WriteDot_(intra, inter):
    from graphviz import Digraph

    # Some constants to control sizing, etc.
    arrow_size   = .5
    font_name    = 'Arial'
    font_size    = 10
    group_margin = 12
    node_height  = .3
    node_sep     = .2
    node_width   = .7
    rank_sep     = .25

    graph_attr = {
        'nodesep'   :  str(node_sep),
        'pencolor'  :  'darkblue',
        'penwidth'  :  '1.2',
        'rankdir'   :  'TB',
        'ranksep'   :  str(rank_sep),
        'splines'   :  'ortho',
    }
    node_attr = {
        'fontcolor' : 'blue',   # They are links
        'fontname'  :  font_name,
        'fontsize'  :  str(font_size),
        'height'    :  str(node_height),
        'margin'    :  '0',
        'shape'     :  'box',
        'style'     :  'rounded',
        'width'     :  str(node_width),
    }
    edge_attr = {
        'arrowsize' : str(arrow_size),
    }
    dot = Digraph(strict=True, graph_attr=graph_attr, node_attr=node_attr,
                  edge_attr=edge_attr)

    # Groups as nested cluster subgraphs. This assumes the order of the groups
    # is the hierarchical ordering. This iterates in reverse order; for some
    # reason, it does not seem to work the other way.
    sub_attr = { 'margin' : str(group_margin) }
    child_cluster = None
    for i, group in reversed(list(enumerate(groups))):
        cluster = Digraph(name=f'cluster_{i}', graph_attr=sub_attr)
        for node in group:
            cluster.node(node, group=str(i), URL=f'\\ref {node}')
        for k, v in intra[i].items():
            for vv in v:
                cluster.edge(k, vv)
        if child_cluster:
            cluster.subgraph(child_cluster)
        child_cluster = cluster
    dot.subgraph(child_cluster)

    print(dot.source)

def main():
    script_dir = dirname(realpath(__file__))
    src_dir = join(script_dir, '../src')
    chdir(src_dir)

    # Get all module-to-module dependencies.
    mod_to_mod = FindModToModDependencies_()

    # Find intra-group module-to-module dependencies for each group. Store in a
    # parallel list. Also find all inter-group dependencies in a dictionary.
    intra = [{} for i in range(len(groups))]
    inter = {}
    for k, v in mod_to_mod.items():
        kg = mod_to_group[k]
        for vv in v:
            vg = mod_to_group[vv]
            if kg == vg:
                AddToDict_(intra[kg], k, vv)
            else:
                AddToDict_(inter, kg, vg)

    # Output the results as dot.
    WriteDot_(intra, inter)

if __name__ == '__main__':
    main()
