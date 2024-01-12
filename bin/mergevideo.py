#!/usr/bin/env python3

from os         import chdir, unlink
from pathlib    import Path
from subprocess import run
from sys        import argv, exit
from tempfile   import NamedTemporaryFile

usage_string = """
Usage:  mergevideo.py OutFile InFile1 InFile2 [...]

  Merges (concatenates) multiple video files (.mp4) and WebVTT chapter files
  (.vtt) into a single MP4 and single WebVTT file. The chapter file times are
  offset appropriately for the merged video.


  Merges (concatenates) video files "InFile1.mp4", "InFile2.mp4", etc. to
  create "OutFile.mp4" and does the same for the WebVTT chapter (.vtt) files.

  Note: at least 2 input files are required.
"""

# -----------------------------------------------------------------------------
# The process is as follows:
#   - Use "ffprobe" to access the duration of each input video.
#   - Compute the actual chapter start/end times.
#   - Use "ffmpeg -f concat" + "-map_metadata" to concatenate the video files
#      and insert the combined chapter metadata.
#   - Write out the combined WebVTT chapter file.
# -----------------------------------------------------------------------------

# XXXXXXXXXXXXXXXXXXXXXX TODO:
#  - Don't use abs path here (or in any script) - get it from cwd
#  - Use WebVTT files for all chapter info - no need for ffprobe
#    - Create both metadata file and merged WebVTT file from them.

# Directory containing the videos.
video_dir = '/home/pss/git/EasyMaker3D/PublicDoc/docs/extra/videos'

# Runs ffprobe with the given arguments (as a string) on the given file,
# returning stdout.
def RunFFProbe_(arg_string, in_file):
    args = ['ffprobe'] + arg_string.split() + [f'{in_file}.mp4']
    # print(f'XXXX RUNNING "{" ".join(args)}"')
    output = run(args, capture_output=True, text=True)
    return output.stdout.strip()

# Returns a dictionary mapping input file name to a duration in seconds.
def GetDurations_(in_files):
    ffprobe_args = ('-v error -show_entries format=duration -of' +
                    ' compact=print_section=0:nokey=1')
    return {in_file: float(RunFFProbe_(ffprobe_args, in_file))
            for in_file in in_files}

# Returns a dictionary mapping input file name to chapter metadata. The
# metadata for each input file is a list of tuples, 1 per chapter:
# (start_time,end_time,title).
def GetChapters_(in_files):
    ffprobe_args = '-v error -show_chapters -of compact=print_section=0'
    chapters = {}
    for in_file in in_files:
        chapter_times = []
        # The output of this command has one chapter per line.
        for chapter_line in RunFFProbe_(ffprobe_args, in_file).split('\n'):
            print(f'XXXX CL={chapter_line}')
            # The output is in the form: "key0=val0|key1=val1|...".
            # Split by the '|' characters and parse the start/end times.
            values = {key: value for key, value in
                      [field.split('=') for field in chapter_line.split('|')]}
            chapter_times.append((float(values['start_time']),
                                  float(values['end_time']),
                                  values['tag:title']))
        chapters[in_file] = chapter_times
    return chapters

# Returns a string containing merged chapter metadata.
def GetMergedChapterMetadata_(in_files, durations, chapters):
    metadata_str = ''
    cur_start = 0
    for in_file in in_files:
        for start_time, end_time, title in chapters[in_file]:
            start_frame = int(1000 * (cur_start + start_time))
            end_frame   = int(1000 * (cur_start + end_time))
            metadata_str += f"""
[CHAPTER]
TIMEBASE=1/1000
START={start_frame}
END={end_frame}
title={title}
"""
        cur_start += durations[in_file]
    return metadata_str

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

    # Step 1: Get video durations using ffprobe.
    durations = GetDurations_(in_files)
    #print(f'XXXX **** durations = {durations}')

    # Step 2: Get chapter start/end times.
    chapters = GetChapters_(in_files)
    #print(f'XXXX **** chapters = {chapters}')

    # Step 3: Construct a merged metadata file to pass to ffmpeg.
    metadata_str = GetMergedChapterMetadata_(in_files, durations, chapters)
    metadata_file = NamedTemporaryFile(mode='w', prefix='mergevideo_',
                                       suffix='.txt', delete=False)
    metadata_file.write(metadata_str)
    metadata_file.close()

    # Step 4: Create a text file to pass to ffmpeg concatenation. Note that
    # these have to be absolute paths, since ffmpeg interprets relative paths
    # based on the directory the concat_file is in.
    concat_file = NamedTemporaryFile(mode='w', prefix='mergevideo_',
                                     suffix='.txt', delete=False)
    for in_file in in_files:
        concat_file.write(f'file {video_dir}/{in_file}.mp4\n')
    concat_file.close()

    # Step 5: Use ffmpeg to concatenate the video files and add the chapter
    # metadata.
    ffmpeg_cmd = (f'ffmpeg -v error -f concat -safe 0 -i {concat_file.name}' +
                  f' -i {metadata_file.name} -map_metadata 1 -c copy -y' +
                  f' {out_file}.mp4')
    # XXXX
    ffmpeg_cmd = (f'ffmpeg -f concat -safe 0 -i {concat_file.name}' +
                  f' -c copy -y' +
                  f' {out_file}.mp4')
    print(f'XXXX RUNNING "{ffmpeg_cmd}"')
    run(ffmpeg_cmd.split())

    # Clean up.
    unlink(concat_file.name)
    unlink(metadata_file.name)

    # XXXX NEED TO WRITE WebVTT file!!!

if __name__ == '__main__':
    main()
