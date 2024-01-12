#!/usr/bin/env python3

from datetime   import datetime, time, timedelta
from os         import chdir, unlink
from pathlib    import Path
from subprocess import run
from sys        import argv, exit
from tempfile   import NamedTemporaryFile
from webvtt     import read as ReadWebVTT

usage_string = """
Usage:  mergevideo.py OutFile InFile1 InFile2 [...]

  Merges (concatenates) input video files ("InFile1.mp4", "InFile2.mp4", ...)
  to create "OutFile.mp4" and does the same for the WebVTT chapter (.vtt)
  files. All chapter times in the output video and WebVTT files are offset
  appropriately for the merged video.

  Notes:
   - This must be run in the top-level project directory.
   - Files are specified relative to the PublicDoc/docs/extra/videos directory.
   - At least 2 input files are required.
"""

# Directory containing the videos.
video_dir = f'{Path.cwd()}/PublicDoc/docs/extra/videos'

# -----------------------------------------------------------------------------
# A Chapter_ instance stores chapter information.
# -----------------------------------------------------------------------------

class Chapter_(object):
    def __init__(self, tag, start_time, end_time, title):
        self.tag        = tag
        self.start_time = start_time
        self.end_time   = end_time
        self.title      = title
    def __repr__(self):
        return f'CH({self.tag}:{self.start_time},{self.end_time},"{self.title}")'
    def Offset(self, offset_time):
        'Returns a new instance offset by the given time.'
        return Chapter_(self.tag,
                        offset_time + self.start_time,
                        offset_time + self.end_time,
                        self.title)

# -----------------------------------------------------------------------------
# Verifies that all input files exist.
# -----------------------------------------------------------------------------

def VerifyInFiles_(in_files):
    all_exist = True

    for in_file in in_files:
        if not Path(in_file + '.mp4').exists():
            print(f'*** Input file {in_file}.mp4 does not exist')
            all_exist = False
        if not Path(in_file + '.vtt').exists():
            print(f'*** Input file {in_file}.vtt does not exist')
            all_exist = False

    if not all_exist:
        exit(1)

# -----------------------------------------------------------------------------
# Parses input WebVTT files, returning a dictionary mapping input
# file name to a list of Chapter_ instances.
# -----------------------------------------------------------------------------

def GetChapterDict_(in_files):
    def StrToSeconds_(s):
        t = time.fromisoformat(s)
        td = timedelta(hours=t.hour, minutes=t.minute, seconds=t.second,
                       microseconds=t.microsecond)
        return td.total_seconds()

    return { in_file: [Chapter_(chapter.identifier,
                                StrToSeconds_(chapter.start),
                                StrToSeconds_(chapter.end),
                                chapter.text)
                       for chapter in ReadWebVTT(Path(in_file + '.vtt'))]
             for in_file in in_files }

# -----------------------------------------------------------------------------
# Returns a list of Chapter_ instances with the actual chapter start/end times
# for the merged video.
# -----------------------------------------------------------------------------

def GetMergedChapters_(in_files, chapter_dict):
    merged_chapters = []
    cur_start = 0
    for in_file in in_files:
        for chapter in chapter_dict[in_file]:
            merged_chapters.append(chapter.Offset(cur_start))
        # The last end_time is assumed to be the duration.
        cur_start += chapter_dict[in_file][-1].end_time
    return merged_chapters

# -----------------------------------------------------------------------------
# Constructs and writes a merged video metadata file to pass to ffmpeg. Returns
# the (temporary) metadata file.
# -----------------------------------------------------------------------------

def WriteMergedMetadataFile_(merged_chapters):
    # Create a temporary file. Do NOT delete when done, since ffmpeg needs to
    # read this. (TODO: After upgrade to Python 3.12, use the new
    # 'delete_on_write' flag instead.
    metadata_file = NamedTemporaryFile(mode='w', prefix='mergevideo_',
                                       suffix='.txt', delete=False)

    # Write the metadata.
    for chapter in merged_chapters:
        start_frame = int(1000 * chapter.start_time)
        end_frame   = int(1000 * chapter.end_time)
        metadata_file.write('[CHAPTER]\nTIMEBASE=1/1000\n')
        metadata_file.write(f'START={start_frame}\n')
        metadata_file.write(f'END={end_frame}\n')
        metadata_file.write(f'TITLE={chapter.title}\n')

    metadata_file.close()
    return metadata_file

# -----------------------------------------------------------------------------
# Constructs and writes a text file listing all source video files to pass to
# ffmpeg concatenation. Note that these have to be absolute paths, since ffmpeg
# interprets relative paths based on the (temporary) directory the concat_file
# is in. Returns the (temporary) list file.
# -----------------------------------------------------------------------------

def WriteVideoListFile_(in_files):
    list_file = NamedTemporaryFile(mode='w', prefix='mergevideo_',
                                   suffix='.txt', delete=False)
    for in_file in in_files:
        list_file.write(f'file {video_dir}/{in_file}.mp4\n')
    list_file.close()
    return list_file

# -----------------------------------------------------------------------------
# Runs ffmpeg to concatenate the video files and write the result to the output
# file.
# -----------------------------------------------------------------------------

def ConcatenateFiles_(metadata_file, list_file, out_file):
    ffmpeg_cmd = (f'ffmpeg -v error -f concat -safe 0 -i {list_file.name}' +
                  f' -i {metadata_file.name} -map_metadata 1 -c copy -y' +
                  f' {out_file}.mp4')
    output = run(ffmpeg_cmd.split(), capture_output=True, text=True)
    if output.stderr:
        print(f'*** Error running ffmpeg: {output.stderr}')

# -----------------------------------------------------------------------------
# Writes the merged chapter data to a WebVTT file.
# -----------------------------------------------------------------------------

def WriteMergedWebVTTFile_(merged_chapters, out_file):
    # Unfortunately, webvtt-py cannot write out in WebVTT format.
    def ToTimeStr_(seconds):
        # Unfortunately, Python timedelta does not provide formatting.
        t = (datetime.min + timedelta(seconds=seconds)).time()
        mils = int(t.microsecond / 1000)
        return t.strftime('%X.') + f'{mils:03}'
    with open(out_file + '.vtt', 'w') as wf:
        wf.write('WEBVTT\n\n')
        for chapter in merged_chapters:
            st = ToTimeStr_(chapter.start_time)
            et = ToTimeStr_(chapter.end_time)
            wf.write(f'{chapter.tag}\n')
            wf.write(f'{st} --> {et}\n')
            wf.write(f'{chapter.title}\n\n')

# -----------------------------------------------------------------------------
# Mainline.
#
# The process is as follows:
#   - Parse the WebVTT files to get chapter times and the video duration for
#      each input video.
#   - Compute the actual chapter start/end times for the merged video.
#   - Use ffmpeg to concatenate the video files and insert the combined chapter
#      metadata.
#   - Write out the combined WebVTT chapter file.
# -----------------------------------------------------------------------------

def main():
    if len(argv) < 4:
        print('*** Not enough arguments\n' + usage_string)
        exit(-1)

    # Change to the video directory to simplify everything.
    chdir(video_dir)

    out_file = argv[1]
    in_files = argv[2:]
    print(f'=== Merging {" ".join(in_files)} => {out_file}')

    # Verify that all input files exist.
    VerifyInFiles_(in_files)

    # Step 1: Parse all input WebVTT files, creating a dictionary mapping input
    # file name to (start_time,end_time,title).
    chapter_dict = GetChapterDict_(in_files)

    # Step 2: Compute the actual chapter start/end times for the merged video.
    merged_chapters = GetMergedChapters_(in_files, chapter_dict)

    # Step 3: Construct and write a merged metadata file to pass to ffmpeg.
    metadata_file = WriteMergedMetadataFile_(merged_chapters)

    # Step 4: Create a text file to pass to ffmpeg concatenation.
    list_file = WriteVideoListFile_(in_files)

    # Step 5: Use ffmpeg to concatenate the video files and add the chapter
    # metadata.
    ConcatenateFiles_(metadata_file, list_file, out_file)

    # Clean up.
    unlink(list_file.name)
    unlink(metadata_file.name)

    # Write the merged WebVTT file.
    WriteMergedWebVTTFile_(merged_chapters, out_file)

if __name__ == '__main__':
    main()
