from docutils             import nodes
from docutils.parsers.rst import Directive

# -----------------------------------------------------------------------------
# MenuIcon directive class.
# -----------------------------------------------------------------------------

class MenuIcon(Directive):
  """MenuIcon inserts an image representing a menu icon at an appropriate size.
  Required arguments: icon_name
  """
  required_arguments = 1

  def run(self):
    icon_name  = self.arguments[0]
    image_node = nodes.image(uri=f'/images/MenuIcons/MI{icon_name}.png',
                             width='28px')
    return [image_node]

# -----------------------------------------------------------------------------
# Extension setup.
# -----------------------------------------------------------------------------

def setup(app):
  app.add_directive("menuicon", MenuIcon)
  return {
    'version': '0.1',
    'parallel_read_safe': True,
    'parallel_write_safe': True,
  }
