#@@@@
# SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
# SPDX-License-Identifier: AGPL-3.0-or-later
#@@@@

from docutils             import nodes, statemachine
from docutils.parsers.rst import Directive, directives

# -----------------------------------------------------------------------------
# IncImage directive class.
# -----------------------------------------------------------------------------

class IncImage(Directive):
  """IncImage includes an image.
  Required arguments: uri size align
  Optional arguments: caption, block [if specified, makes figure a block]

  If size is positive, it is used as the width; if negative, it is used as the
  height.
  """
  required_arguments = 3
  optional_arguments = 2

  # Set up optional arguments.
  option_spec = {
    'caption': directives.unchanged,
    'block':   directives.flag,
  }

  def run(self):
    (uri, size, align) = self.arguments
    caption = self.options.get('caption', None)
    block   = 'block' in self.options
    node_list = [self._BuildFigure(uri, size, align)]
    if caption:
      node_list.append(self._BuildCaption(caption, align))
    if block:
      node_list.append(self._BuildBlock())
    return node_list

  def _BuildFigure(self, uri, size, align):
    image_node  = (nodes.image(uri=uri, width=size) if size[0] != '-' else
                   nodes.image(uri=uri, height=size[1:]))
    figure_node = nodes.figure('', image_node)
    figure_node['align'] = align
    figure_node['classes'] = [f'align-{align}']
    return figure_node

  def _BuildCaption(self, caption, align):
    caption_node = nodes.paragraph()
    # Handle substitutions in the caption text.
    self.state.nested_parse(statemachine.ViewList([caption], 'caption'),
                            0, caption_node)
    # For some reason, the nested_parse() call adds the substituted text as an
    # extra level.
    caption_node = caption_node[0]
    caption_node['classes'] = [f'caption-{align}']
    return caption_node

  def _BuildBlock(self):
    block_node = nodes.paragraph(text=u'\u00a0')  # NBSP
    block_node['classes'] = ['after-image']
    return block_node

# -----------------------------------------------------------------------------
# Extension setup.
# -----------------------------------------------------------------------------

def setup(app):
  app.add_directive("incimage", IncImage)
  return {
    'version': '0.1',
    'parallel_read_safe': True,
    'parallel_write_safe': True,
  }
