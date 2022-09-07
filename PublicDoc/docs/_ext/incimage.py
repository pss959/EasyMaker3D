from docutils             import nodes, statemachine
from docutils.parsers.rst import Directive, directives

# -----------------------------------------------------------------------------
# IncImage directive class.
# -----------------------------------------------------------------------------

class IncImage(Directive):
  """IncImage includes an image.
  Required arguments: uri width align
  Optional arguments: caption, block [if specified, makes figure a block]
  """
  required_arguments = 3
  optional_arguments = 2

  # Set up optional arguments.
  option_spec = {
    'caption': directives.unchanged,
    'block':   directives.flag,
  }

  def run(self):
    (uri, width, align) = self.arguments
    caption = self.options.get('caption', None)
    block   = 'block' in self.options
    node_list = [self.BuildFigure_(uri, width, align)]
    if caption:
      node_list.append(self.BuildCaption_(caption, align))
    if block:
      node_list.append(self.BuildBlock_())
    return node_list

  def BuildFigure_(self, uri, width, align):
    image_node  = nodes.image(uri=uri, width=width)
    figure_node = nodes.figure('', image_node)
    figure_node['align'] = align
    figure_node['classes'] = [f'align-{align}']
    return figure_node

  def BuildCaption_(self, caption, align):
    caption_node = nodes.paragraph()
    # Handle substitutions in the caption text.
    self.state.nested_parse(statemachine.ViewList([caption], 'caption'),
                            0, caption_node)
    # For some reason, the nested_parse() call adds the substituted text as an
    # extra level.
    caption_node = caption_node[0]
    caption_node['classes'] = [f'caption-{align}']
    return caption_node

  def BuildBlock_(self):
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
