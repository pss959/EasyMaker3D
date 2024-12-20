#!/usr/bin/env python3

#@@@@
# SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
# SPDX-License-Identifier: AGPL-3.0-or-later
#@@@@

from glob    import glob
from os      import popen, remove, replace, system
from os.path import basename, splitext
from sys     import argv

# This defines how many images are in each row of the atlas.
kColCount = 8

# -----------------------------------------------------------------------------
# Guarantees that all PNG images in the list are the correct size (128x128) and
# format.
# -----------------------------------------------------------------------------

def FixImageFiles(image_files):
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

# -----------------------------------------------------------------------------
# Creates the texture atlas PNG file.
# -----------------------------------------------------------------------------

def CreateTextureAtlas(image_files, out_png_file):
  # Use ImageMagick to create the atlas.
  file_list = ' '.join(image_files)
  cmd = (f'montage -mode concatenate -background None -tile {kColCount}x'
         f' {file_list} images/{out_png_file}')
  system(cmd)

# -----------------------------------------------------------------------------
# Returns a triple containing (texture_name, scale, offset) for each image.
# The scale and offset are both (x,y) pairs.
# -----------------------------------------------------------------------------

def GetTextureTriples(image_files):
  from math import ceil

  # X scale is constant. There is always space for kColCount images across.
  scale_x = 1. / kColCount

  # Y scale depends on how many rows there are.
  row_count = ceil(len(image_files) / kColCount)
  scale_y = 1. / row_count

  row = 0
  col = 0

  triples = []
  for im in image_files:
    # The uniform name is the name of the file (without the extension).
    name = splitext(basename(im))[0]

    # Y coordinates have 1 at the top, so do not reverse them.
    offset_x = 1 - (col * scale_x)
    offset_y = (row + 1) * scale_y

    triples.append((name, (scale_x, scale_y), (offset_x, offset_y)))

    col = col + 1
    if col == kColCount:
      col = 0
      row = row + 1

  return triples

# -----------------------------------------------------------------------------
# Creates the TextureAtlas node file.
# -----------------------------------------------------------------------------

def CreateNodeFile(image_files, out_png_file, out_mvn_file, image_name):
  # Get triples for all textures: (name, scale, offset)
  triples = GetTextureTriples(image_files)

  header = (
f"""# This file was generated by {basename(argv[0])}
FileImage "{image_name}" {{
  path: "{out_png_file}",
  is_static: True,
  sub_images: [
""")

  sub_images = ''
  for triple in triples:
    name = triple[0]
    sx   = triple[1][0]
    sy   = triple[1][1]
    ox   = triple[2][0]
    oy   = triple[2][1]
    sub_images += (
f"""    SubImage "{name}" {{
      texture_scale:  {sx} {sy},
      texture_offset: {ox} {oy},
    }},
""")

  with open(out_mvn_file, 'w') as f:
    f.write(header)
    f.write(sub_images)
    f.write('  ]\n')
    f.write('}\n')

# -----------------------------------------------------------------------------
# Mainline.
# -----------------------------------------------------------------------------

usage = f"""
*** Usage: {argv[0]} <in_png_dir> <out_png_file> <out_mvn_file> <image_name>
    Creates a texture atlas from a set of images. This results in a PNG file
    containing all of the input images arranged in a grid and an MVN file
    containing a singe FileImage definition with all of the input images
    specified as sub-images.

    <in_png_dir>:
        Directory containing the input images, relative to the resources
        directory. All .png files in this directory are added to the atlas.
    <out_png_file>:
        The path of the output PNG file to contain the texture atlas, relative
        to the resources/images subdirectory.
    <out_mvn_file>
       The path of the output MVN file to contain the FileImage instance,
       relative to the resources directory.
    <image_name>
       The name to give to the FileImage instance.
"""

def main():
  if len(argv) != 5:
    print(usage)
    return 1

  in_png_dir   = argv[1]
  out_png_file = argv[2]
  out_mvn_file = argv[3]
  image_name   = argv[4]

  # Make sure there are images.
  image_files = sorted(glob(f'{in_png_dir}/*.png'))
  if not image_files:
    print(f'*** No PNG images found in {in_png_dir}')
    return 2

  FixImageFiles(image_files)
  CreateTextureAtlas(image_files, out_png_file)
  CreateNodeFile(image_files, out_png_file, out_mvn_file, image_name)

if __name__ == '__main__':
  main()
