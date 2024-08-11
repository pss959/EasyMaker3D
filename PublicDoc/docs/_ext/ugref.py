#@@@@
# SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
# SPDX-License-Identifier: AGPL-3.0-or-later
#@@@@

from docutils import nodes

# -----------------------------------------------------------------------------
# UGButtonNode directive class.
# -----------------------------------------------------------------------------

class UGButtonNode(nodes.General, nodes.Element):
    'UGButtonNode inserts a button that has the ug-button class'
    pass

def EnterUGButtonNode(translator, node):
    translator.body.append('<button class="ug-button"></button>')

def ExitUGButtonNode(translator, node):
    pass

# -----------------------------------------------------------------------------
# Implements the ugref role.
#
# This is a 2-step process. First, the 'ugref' role adds the 'ug-ref' class to
# the cross-reference node to resolve. Then, after references in the doctree
# have been resolved, this replaces the anchor text with a UGButtonNode().
# -----------------------------------------------------------------------------

def ugref_role(name, rawtext, text, lineno, inliner, options={}, content=None):
    # Run the standard function for the 'ref' role. [If anyone cares, this was
    # super difficult to figure out.]
    domain  = inliner.document.settings.env.domains['std']
    role_fn = getattr(domain, 'role')('ref')
    nodes, messages = role_fn('ref', rawtext, text, lineno, inliner)

    # Add the ug-ref class to the returned node so it can be found later.
    nodes[0].set_class('ug-ref')

    return nodes, messages

def process_ugref_nodes(app, doctree, fromdocname):
    # Find each reference node with the 'ug-ref' class and replace its child
    # with a UGButtonNode.
    for node in doctree.findall(nodes.reference):
        if 'ug-ref' in node.get('classes'):
            node.replace(node.children[0], UGButtonNode())

# -----------------------------------------------------------------------------
# Extension setup.
# -----------------------------------------------------------------------------

def setup(app):
    app.add_node(UGButtonNode, html=(EnterUGButtonNode, ExitUGButtonNode))
    app.add_role("ugref", ugref_role)
    app.connect('doctree-resolved', process_ugref_nodes)
    return {
        'version': '0.1',
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }
