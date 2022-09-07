from docutils             import nodes, statemachine
from docutils.parsers.rst import Directive, directives

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
    uri   = self.arguments[0]
    width = self.arguments[1]
    align = self.arguments[2]
    caption = self.options.get('caption', None)
    block   = 'block' in self.options
    image_node = nodes.image(uri=uri, width=width)
    figure_node = nodes.figure()
    figure_node.set_class(f'align-{align}')
    figure_node.append(image_node)
    node_list = [figure_node]
    if caption:
      caption_node = nodes.paragraph()
      # Handle substitutions in the caption text.
      self.state.nested_parse(statemachine.ViewList([caption], 'caption'),
                              0, caption_node)
      caption_node.set_class(f'caption-{align}')
      node_list.append(caption_node)
    if block:
      block_node = nodes.paragraph(text='')
      block_node.set_class('after-image')
      node_list.append(block_node)
    print(f'XXXX Returning node_list: {node_list}')
    return node_list

def setup(app):
  app.add_directive("incimage", IncImage)
  return {
    'version': '0.1',
    'parallel_read_safe': True,
    'parallel_write_safe': True,
  }
