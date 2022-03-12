#!/usr/bin/env python3

from glob import glob
from os   import popen, remove, replace, system
from sys  import argv

# Creates a texture atlas from a set of images.
#
# Command-line arguments:
#   <image_dir>
#       Directory containing the images. All .png files in this directory are
#       added to the atlas.
#   <output_png_path>
#       The path of the output file to contain the texture atlas.
#   <output_node_path>
#       The path of the output file to contain the MVN TextureAtlas node.

# -----------------------------------------------------------------------------
# Mainline.
# -----------------------------------------------------------------------------

usage = f"""*** Usage: {argv[0]} <image_dir> <atlas_path> <node_path>
    <image_dir>:
        Directory containing the images. All .png files in this directory are
        added to the atlas.
    <atlas_path>:
        The path of the output file to contain the texture atlas relative to
        the current directory.
    <node_path>
       The path of the output file to contain the MVN TextureAtlas node
       relative to the current directory.
"""

def main():
  if len(argv) != 4:
    print(usage)
    return 1

  image_dir  = argv[1]
  atlas_path = argv[2]
  node_path  = argv[3]


  # Make sure there are images.
  image_files = glob(f'{image_dir}/*.png')
  if not image_files:
    print(f'*** No PNG images found in {image_dir}')
    return 2

  # Guarantee that all PNG images are the correct size (128x128) and format.
  for im in image_files:
    tmpfile = 'tmp.png'
    system(f'convert -define colorspace:auto-grayscale=off'
           f' -resize 128x128! {im} PNG32:{tmpfile}')
    # Check for differences before overwriting.
    sha1 = popen(f'identify -quiet -format "%#" {im}').read()
    sha2 = popen(f'identify -quiet -format "%#" {tmpfile}').read()
    if sha1 == sha2:
      remove(tmpfile)
    else:
      replace(tmpfile, im)

    # Use ImageMagick to create the atlas.
    file_list = ' '.join(image_files)
    cmd = ('montage -mode concatenate -background None -tile 16x'
           f' {file_list} {atlas_path}')
    system(cmd)

if __name__ == '__main__':
  main()
