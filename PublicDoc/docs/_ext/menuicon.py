#@@@@
# SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
# SPDX-License-Identifier: AGPL-3.0-or-later
#@@@@

from docutils import nodes

# -----------------------------------------------------------------------------
# Implements the menuicon role.
# -----------------------------------------------------------------------------

def menuicon_role(name, rawtext, text, lineno, inliner,
                  options=None, content=None):
    'Inserts an image node representing a menu icon at an appropriate size.'
    try:
        if not text:
            raise ValueError
    except ValueError:
        msg = inliner.reporter.error('Missing name for menuicon role')
        prb = inliner.problematic(rawtext, rawtext, msg)
        return [prb], [msg]
    icon_name = text
    node = nodes.image(uri=f'/images/MenuIcons/MI{icon_name}.png', width='28px')
    return [node], []

# -----------------------------------------------------------------------------
# Extension setup.
# -----------------------------------------------------------------------------

def setup(app):
  app.add_role("menuicon", menuicon_role)
  return {
    'version': '0.1',
    'parallel_read_safe': True,
    'parallel_write_safe': True,
  }
