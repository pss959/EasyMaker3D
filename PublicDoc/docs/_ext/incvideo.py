from docutils             import nodes
from docutils.parsers.rst import Directive, directives
from pathlib              import Path
from sphinx               import application
from sphinx.util.osutil   import relative_uri

# This dictionary mapping video ID to source file is shared with vidref.py so
# it can figure out where to get the corresponding VTT file.
video_dict = {}

# -----------------------------------------------------------------------------
# VideoNode class.
# -----------------------------------------------------------------------------

# There is no docutils node corresponding to a video, so create one here.
class VideoNode(nodes.General, nodes.Element):
    pass

# -----------------------------------------------------------------------------
# IncVideo directive class.
# -----------------------------------------------------------------------------

class IncVideo(Directive):
    """IncVideo adds an embedded video with a corresponding chapters file.
    Required arguments: id
    Optional arguments: width [default 90%] align [default center]

    The ID can be used in videobutton to start the video at a certain time.
    """
    has_content        = True
    required_arguments = 1
    optional_arguments = 2

    # Set up optional arguments.
    option_spec = {
        'width': directives.unchanged,
        'align': directives.unchanged,
    }

    def run(self):
        id    = self.arguments[0]
        width = self.options.get('width', '90%')
        align = self.options.get('align', 'center')
        # Construct the URI from the ID.
        uri   = f'../videos/{id}/{id}.mp4'
        # Save the mapping from video ID to source file.
        video_dict[id] = uri
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
    return {
      'version': '0.1',
      'parallel_read_safe':  True,
      'parallel_write_safe': True,
    }
