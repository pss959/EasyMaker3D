from docutils             import nodes
from docutils.parsers.rst import Directive, directives
from pathlib              import Path
from sphinx.errors        import ExtensionError

# This maps video ID to a source file path.
from incvideo import video_dict

# -----------------------------------------------------------------------------
# Returns the start time for a section name in a video.
# -----------------------------------------------------------------------------

def SectionStartTime_(video_path, section):
    # Read the corresponding WebVTT chapter file. Note that this script runs in
    # the PublicDoc directory.
    import webvtt
    from datetime import datetime, timedelta
    file_name = Path(f'./docs/extra/videos/{video_path}').with_suffix('.vtt')
    for chapter in webvtt.read(file_name):
        if chapter.identifier == section:
            dt = datetime.strptime(chapter.start, '%H:%M:%S.%f')
            td = timedelta(hours=dt.hour, minutes=dt.minute, seconds=dt.second,
                           microseconds=dt.microsecond)
            return td.total_seconds()
    # The chapter tag was not found.
    raise ExtensionError('***Chapter tag "' + section + '" was not found in "' +
                         str(file_name) + '"', modname="vidref.py")

# -----------------------------------------------------------------------------
# VideoButtonNode class.
# -----------------------------------------------------------------------------

class VideoButtonNode(nodes.General, nodes.Element):
    """VideoButtonNode inserts a button that plays a particular video starting
    at a given time in seconds.
    Required dictionary contents: 'video_id' 'section'
    """
    pass

def EnterVideoButtonNode(translator, node):
    # Access node items.
    video_id = node['video_id']
    section  = node['section']
    if not video_id in video_dict:
        raise ExtensionError('***Video ID "' + video_id + '" was not found',
                             modname="vidref.py")
    video_path = video_dict[video_id]
    start_time = SectionStartTime_(video_path, section)
    html = (f'<button class="video-button" data-id="{video_id}"' +
            f' data-start-time="{start_time}"></button>')
    translator.body.append(html)

def ExitVideoButtonNode(translator, node):
    pass

# -----------------------------------------------------------------------------
# Implements the VidRef role.
# -----------------------------------------------------------------------------

def vidref_role(name, rawtext, text, lineno, inliner,
                options=None, content=None):
    """Inserts a button referring to a specific time in a video. Expects text
    containing the video name and section name, separated by a colon.
    """
    try:
        if not text:
            raise ValueError
        video, section = text.split(':')
    except ValueError:
        msg = inliner.reporter.error('Bad syntax for vidref role')
        prb = inliner.problematic(rawtext, rawtext, msg)
        return [prb], [msg]
    node = VideoButtonNode(video_id=video, section=section)
    return [node], []

# -----------------------------------------------------------------------------
# Extension setup.
# -----------------------------------------------------------------------------

def setup(app):
    app.add_node(VideoButtonNode, html=(EnterVideoButtonNode,
                                        ExitVideoButtonNode))
    app.add_role("vidref", vidref_role)
    return {
        'version': '0.1',
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }
