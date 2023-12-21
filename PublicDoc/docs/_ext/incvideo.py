from docutils             import nodes
from docutils.parsers.rst import Directive, directives
from pathlib              import Path
from sphinx               import application
from sphinx.util.osutil   import relative_uri

# -----------------------------------------------------------------------------
# IncVideo directive class.
# -----------------------------------------------------------------------------

# There is no docutils node corresponding to a video, so create one here.
class VideoNode(nodes.General, nodes.Element):
    pass

_app = None

class IncVideo(Directive):
    """IncVideo adds an embedded video with a corresponding chapters file.
    Required arguments: id uri width align

    The ID can be used in videobutton to start the video at a certain time.
    """
    has_content        = True
    required_arguments = 4

    def run(self):
        global _app
        (id, uri, width, align) = self.arguments
        return [VideoNode(id=id, source=uri, width=width, align=align)]

def EnterVideoNode(translator, node):
    # Access node items.
    id     = node['id']
    width  = node['width']
    source = node['source']
    align  = node['align']

    # Get the video type and the name of the chapters track WebVTT file from
    # the source file name.
    path = Path(source)
    video_type = path.suffix[1:]  # Remove the '.'.
    chapters   = path.with_suffix('.vtt')

    # Create a div containing the video and chapter bar elements. Note that
    # preload="auto" is required in the video to get the duration metadata to
    # be known in time. And setting onloadedmetadata here ensures that it is
    # registered before DOMContentLoaded is called. (Using an event listener in
    # the JavaScript file DOES NOT ALWAYS WORK!!!
    translator.body.append(
        f'<div class="embedded-video" style="width:{width}">\n' +
        f'  <video id="{id}" controls preload="auto"' +
        f' onloadedmetadata="addChapters()" >\n' +
        f'    <source src="{source}" type="video/{video_type}">\n' +
        f'    <track src="{chapters}" kind="chapters" default>\n' +
        f'  </video>\n' +
        f'  <div id="chapterbar"></div>\n' +
        f'</div>\n'
    )

def ExitVideoNode(translator, node):
    pass

# -----------------------------------------------------------------------------
# Extension setup.
# -----------------------------------------------------------------------------

def setup(app):
    """Add the VideoNode to the Sphinx builder."""
    app.add_node(VideoNode, html=(EnterVideoNode, ExitVideoNode))
    app.add_directive("incvideo", IncVideo)
    global _app
    _app = app
    return {
      'version': '0.1',
      'parallel_read_safe':  True,
      'parallel_write_safe': True,
    }
