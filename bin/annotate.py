#!/usr/bin/env python3

from PIL import Image, ImageDraw, ImageFont
from sys import argv

# Scripts are assumed to be relative to this directory:
script_dir = 'PublicDoc/snaps/annotations'

# Images are assumed to be relative to this directory:
image_dir = 'PublicDoc/docs/images'

# -----------------------------------------------------------------------------
# ImageAnnotator class that does most of the work.
# -----------------------------------------------------------------------------

class ImageAnnotator(object):
  def __init__(self):
    self._image = None
    self._draw  = None
    self._color = "white"
    self._font  = ImageFont.truetype('arialbd.ttf', 10)

  def ReadImage(self, image_file):
    try:
      self._image = Image.open(image_file)
      self._draw  = ImageDraw.Draw(self._image)
    except:
      print(f'*** Unable to read image from file "{image_file}"')
      return False
    return True

  def SetColor(self, color):
    self._color = color

  def SetFont(self, font, size):
    self._font = ImageFont.truetype(font + '.ttf', size)

  def DrawRectangle(self, line_width, x, y, w, h):
    if not self._draw:
      print('*** No image read for annotating rectangle')
      return False
    # As of Pillow 9.5.0, coordinates must be in order
    x0, y0 = self.ToImageCoords_(x,   y)
    x1, y1 = self.ToImageCoords_(x+w, y+h)
    if x0 > x1:
        x0, x1 = x1, x0
    if y0 > y1:
        y0, y1 = y1, y0
    self._draw.rectangle([x0, y0, x1, y1],
                         outline = self._color, width=int(line_width))
    return True

  def DrawText(self, x, y, align, text):
    if not self._draw:
      print('*** No image read for annotating text')
      return False
    self._draw.text(self.ToImageCoords_(x, y),
                    text, fill=self._color, font=self._font,
                    align=align, anchor='mm')
                    # Draw outline in white.
                    #stroke_width=1, stroke_fill='#ffffff')
    return True

  def WriteImage(self, image_file):
    if not self._image:
      print('*** No image read to write out')
      return False
    try:
      self._image.save(image_file)
    except:
      print(f'*** Unable to write image to file "{image_file}"')
      return False
    return True

  def ToImageCoords_(self, x, y):
    # Invert Y: PIL has (0,0) at the upper left.
    return (round(x * self._image.width),
            round((1 - y) * self._image.height))

# -----------------------------------------------------------------------------
# Script command processing.
# -----------------------------------------------------------------------------

# Takes 1 argument: color
def ProcessColor(ia, args):
  if len(args) != 1:
    print(f'*** Invalid color arguments: {args}')
    return False
  ia.SetColor(args[0])
  return True

# Takes 2 arguments: font, size
def ProcessFont(ia, args):
  if len(args) != 2:
    print(f'*** Invalid font arguments: {args}')
    return False
  ia.SetFont(args[0], int(args[1]))
  return True

# Takes 1 argument: the image file.
def ProcessRead(ia, args):
  if len(args) != 1:
    print(f'*** Invalid read arguments: {args}')
    return False
  return ia.ReadImage(f'{image_dir}/{args[0]}')

# Takes 5 arguments: line_width, x, y, w, h
def ProcessRect(ia, args):
  if len(args) != 5:
    print(f'*** Invalid rect arguments: {args}')
    return False
  floats = [float(arg) for arg in args]
  return ia.DrawRectangle(floats[0], floats[1], floats[2], floats[3], floats[4])

# Takes 4+ arguments: x y align text...
def ProcessText(ia, args):
  if len(args) < 4:
    print(f'*** Invalid text arguments: {args}')
    return False
  return ia.DrawText(float(args[0]),
                     float(args[1]), args[2], ' '.join(args[3:]))

# Takes 1 argument: the image file.
def ProcessWrite(ia, args):
  if len(args) != 1:
    print(f'*** Invalid write arguments: {args}')
    return False
  return ia.WriteImage(f'{image_dir}/{args[0]}')

def ProcessCommand(ia, command):
    words = command.split()
    cmd   = words[0]
    args  = words[1:]
    ok = False;
    if   cmd == 'color':
      ok = ProcessColor(ia, args)
    elif cmd == 'font':
      ok = ProcessFont(ia, args)
    elif cmd == 'read':
      ok = ProcessRead(ia, args)
    elif cmd == 'rect':
      ok = ProcessRect(ia, args)
    elif cmd == 'text':
      ok = ProcessText(ia, args)
    elif cmd == 'write':
      ok = ProcessWrite(ia, args)
    else:
      print(f'*** Unknown command: "{cmd}')
    return ok

# -----------------------------------------------------------------------------
# Reads the annotation script and returns a list of commands to execute. Prints
# messages and returns None if there are any script errors.
# -----------------------------------------------------------------------------

def ReadScript(script_file):
  try:
    with open(script_file, 'r') as f:
      lines = f.readlines()
  except:
    print(f'*** Unable to read script from file "{script_file}"')
    return None
  commands = []
  for line in lines:
    # Ignore comments and blank lines.
    sline = line.strip()
    if sline and not sline.startswith('#'):
      commands.append(sline)
  return commands

# -----------------------------------------------------------------------------
# Mainline.
# -----------------------------------------------------------------------------

usage = f"""
*** Usage: {argv[0]} <script_file>

Reads an annotation script to produce annotated versions of images.

An annotation script may contain any number of lines with one of the following:

  [whitespace]            => Blank lines are ignored.
  # ...                   => Comment (ignored).
  read image_file         => Reads the image to annotate from a file.
  color <color>           => Sets the color for text and rectangles.
  font name size          => Sets the font to use for text.
  rect line_width x y w h => Draws a rectangle.
  text x y align <text>   => Draws text. align is one of (left, center, right).
  write image_file        => Writes the current result to a file.

Note that the script file is assumed to be relative to the {script_dir}
directory and all image files are assumed to be relative to the {image_dir}
directory.
"""

def main():
  if len(argv) != 2:
    print(usage)
    return 1

  # Read the script.
  script_file = argv[1]
  commands = ReadScript(f'{script_dir}/{script_file}')
  if not commands:
    return False

  # Process the commands.
  ia = ImageAnnotator()
  for command in commands:
    if not ProcessCommand(ia, command):
      return False

  return True

if __name__ == '__main__':
  main()
