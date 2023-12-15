from docutils             import nodes
from docutils.parsers.rst import Directive, directives
from pathlib              import Path

# -----------------------------------------------------------------------------
# IncVideo directive class.
# -----------------------------------------------------------------------------

# There is no docutils node corresponding to a video, so create one here.
class VideoNode(nodes.General, nodes.Element):
    pass

class IncVideo(Directive):
    """IncVideo adds an embedded video with a corresponding chapters file.
    Required arguments: id uri height align
    Optional arguments: poster

    The ID can be used in videobutton to start the video at a certain time.
    """
    has_content        = True
    required_arguments = 4
    optional_arguments = 1

    # Set up optional arguments.
    option_spec = {
        'poster': directives.unchanged,
    }

    def run(self):
        (id, uri, height, align) = self.arguments
        poster = self.options.get('poster', None)
        video_node = VideoNode(id=id, source=uri, height=height, poster=poster)
        return [self.BuildFigure_(video_node, align)]

    def BuildFigure_(self, video_node, align):
        figure_node = nodes.figure('', video_node)
        figure_node['align'] = align
        figure_node['classes'] = [f'align-{align}']
        return figure_node

def EnterVideoNode(translator, node):
    # Access node items.
    id     = node['id']
    height = node['height']
    source = node['source']
    poster = node['poster']

    # Get the video type and the name of the chapters track WebVTT file from
    # the source file name.
    path = Path(source)
    video_type = path.suffix[1:]  # Remove the '.'.
    chapters   = path.with_suffix('.vtt')

    # Video element.
    poster_spec = f' poster="{poster}"' if poster else ''
    html = ('<video controls="True" class="embedded-video"' +
            f' id="{id}" height="{height}"{poster_spec}>\n')

    # Source element.
    html += f'<source src="{source}" type="video/{video_type}">\n'

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
    """Add the VideoNode to the Sphinx builder."""
    app.add_node(VideoNode, html=(EnterVideoNode, ExitVideoNode))
    app.add_directive("incvideo", IncVideo)
    return {
      'version': '0.1',
      'parallel_read_safe':  True,
      'parallel_write_safe': True,
    }
