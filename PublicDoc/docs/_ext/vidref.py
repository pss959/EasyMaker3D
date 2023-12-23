from docutils             import nodes
from docutils.parsers.rst import Directive, directives

# -----------------------------------------------------------------------------
# VideoButtonNode directive class.
# -----------------------------------------------------------------------------

class VideoButtonNode(nodes.General, nodes.Element):
    """VideoButtonNode inserts a button that plays a particular video starting
    at a given time in seconds.
    Required dictionary contents: 'video_id' 'start_time'
    """

def EnterVideoButtonNode(translator, node):
    # Access node items.
    video_id   = node['video_id']
    start_time = node['start_time']
    html = (f'<button class="video-button" data-id="{video_id}"' +
            f' data-start-time="{start_time}"></button>')
    translator.body.append(html)

def ExitVideoButtonNode(translator, node):
    pass

# -----------------------------------------------------------------------------
# Returns the start time for a section name in a video.
# -----------------------------------------------------------------------------

def SectionStartTime(video, section):
    # Read the corresponding WebVTT chapter file. Note that this script runs in
    # the PublicDoc directory.
    import webvtt
    from datetime import datetime, timedelta
    for chapter in webvtt.read(f'./docs/extra/videos/{video}.vtt'):
        if chapter.identifier == section:
            dt = datetime.strptime(chapter.start, '%H:%M:%S.%f')
            td = timedelta(hours=dt.hour, minutes=dt.minute, seconds=dt.second)
            return td.total_seconds()

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
    start_time = SectionStartTime(video, section)
    node = VideoButtonNode(video_id=video, start_time=start_time)
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
