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
        global _app
        (id, uri, height, align) = self.arguments
        video_node = VideoNode(id=id, source=uri, height=height, poster=None)
        ret_nodes = [self.BuildFigure_(video_node, align)]

        # Special processing for poster image to set up the proper relative URI
        # and a file dependency.
        poster = self.options.get('poster', None)
        if poster:
            # Add an image node so Sphinx dependencies are set up correctly.
            image_node = nodes.image(uri=poster)
            # There does not appear to be a way to hide an image via docutils,
            # so add a class and do it in CSS.
            image_node.set_class('poster-image')
            ret_nodes.append(image_node)

            # Convert the poster URI to a relative target for the attribute.
            path = Path(poster)
            rel_path = path.relative_to(*path.parts[:2])
            video_node['poster'] = relative_uri(_app.env.docname,
                                                '_images') / rel_path

        return ret_nodes

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
    global _app
    _app = app
    return {
      'version': '0.1',
      'parallel_read_safe':  True,
      'parallel_write_safe': True,
    }
