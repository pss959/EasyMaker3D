from docutils             import nodes
from docutils.parsers.rst import Directive, directives

# -----------------------------------------------------------------------------
# VideoButton directive class.
# -----------------------------------------------------------------------------

# There is no docutils node corresponding to a button, so create one here.
class VideoButtonNode(nodes.General, nodes.Element):
    pass

class VideoButton(Directive):
    """VideoButton inserts a button that plays a particular video starting at a
    given time in seconds.
    Required arguments: video_id seconds text
    """
    required_arguments = 3

    def run(self):
        (video_id, seconds, text) = self.arguments
        return [VideoButtonNode(video_id=video_id, seconds=seconds, text=text)]

def EnterVideoButtonNode(translator, node):
    # Access node items.
    video_id = node['video_id']
    seconds  = node['seconds']
    text     = node['text']

    # Video element.
    html = (f'<button class="video-button" data-id="{video_id}"' +
            f' data-seconds="{seconds}">{text}</button>')
    translator.body.append(html)

def ExitVideoButtonNode(translator, node):
    pass

# -----------------------------------------------------------------------------
# Extension setup.
# -----------------------------------------------------------------------------

def setup(app):
    """Add the VideoButtonNode to the Sphinx builder."""
    app.add_node(VideoButtonNode, html=(EnterVideoButtonNode,
                                        ExitVideoButtonNode))
    app.add_directive("videobutton", VideoButton)
    return {
      'version': '0.1',
      'parallel_read_safe': True,
      'parallel_write_safe': True,
    }
