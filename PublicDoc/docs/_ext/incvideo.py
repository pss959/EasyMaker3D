from docutils             import nodes
from docutils.parsers.rst import Directive, directives
from pathlib              import Path

# -----------------------------------------------------------------------------
# IncVideo directive class.
# -----------------------------------------------------------------------------

class VideoNode(nodes.General, nodes.Element):
    pass

class IncVideo(Directive):
    """IncVideo adds an embedded video with a corresponding chapters file.
    Required arguments: uri height
    """
    has_content        = True
    required_arguments = 2
    optional_arguments = 0

    # Set up optional arguments.
    option_spec = {}

    def run(self):
        (uri, height) = self.arguments
        return [VideoNode(source=uri, height=height)]

def EnterVideoNode(translator, node):
    # Access node items.
    height = node['height']
    source = node['source']

    # Get the name of the chapters track WebVTT file.
    chapters = Path(source).with_suffix('.vtt')

    # Video element.
    html = f'<video controls="True" class="embedded-video" height={height}>\n'

    # Source element.
    html += f'<source src="{source}" type="video/webm">\n'

    # Chapter track element.
    html += (f'<track src="{chapters}" kind="chapters"' +
             ' srclang="en" label="English">')

    translator.body.append(html)

def ExitVideoNode(translator, node):
    translator.body.append('</video>\n')

# -----------------------------------------------------------------------------
# Extension setup.
# -----------------------------------------------------------------------------

def setup(app):
    """Add the VideoNode_ to the Sphinx builder."""
    app.add_node(VideoNode, html=(EnterVideoNode, ExitVideoNode))
    app.add_directive("incvideo", IncVideo)
    return {
      'version': '0.1',
      'parallel_read_safe':  True,
      'parallel_write_safe': True,
    }
