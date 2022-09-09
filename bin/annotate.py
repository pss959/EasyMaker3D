#!/usr/bin/env python3

from PIL import Image, ImageDraw, ImageFont
from sys import argv

class ImageAnnotator(object):
  def __init__(self, image):
    self._image = image
    self._draw  = ImageDraw.Draw(self._image)
    self._color = "white"
    self._font  = ImageFont.truetype('arialbd.ttf', 10)

  def SetColor(self, color):
    self._color = color

  def SetFont(self, font, size):
    self._font = ImageFont.truetype(font + '.ttf', size)

  def DrawRectangle(self, line_width, x, y, w, h):
    self._draw.rectangle([self.ToImageCoords_(x, y),
                          self.ToImageCoords_(x+w, y+h)],
                         outline = self._color, width=int(line_width))

  def DrawText(self, x, y, align, text):
    self._draw.text(self.ToImageCoords_(x, y),
                    text, fill=self._color, font=self._font,
                    align=align, anchor='mm')
                    # Draw outline in white.
                    #stroke_width=1, stroke_fill='#ffffff')

  def Save(self, image_file):
    try:
      self._image.save(image_file)
    except:
      return False
    return True

  def ToImageCoords_(self, x, y):
    # Invert Y: PIL has (0,0) at the upper left.
    return (round(x * self._image.width),
            round((1 - y) * self._image.height))

# -----------------------------------------------------------------------------
# Reads the given input image and returns a PIL Image if successful, Prints an
# error message and returns None if not.
# -----------------------------------------------------------------------------

def ReadImage(image_file):
  try:
    image = Image.open(image_file)
  except:
    print(f'*** Unable to read image from file "{image_file}"')
    image = None
  return image

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

# Takes 5 arguments: line_width, x, y, w, h
def ProcessRect(ia, args):
  if len(args) != 5:
    print(f'*** Invalid rect arguments: {args}')
    return False
  floats = [float(arg) for arg in args]
  ia.DrawRectangle(floats[0], floats[1], floats[2], floats[3], floats[4])
  return True

# Takes 4+ arguments: x y align text...
def ProcessText(ia, args):
  if len(args) < 4:
    print(f'*** Invalid text arguments: {args}')
    return False
  ia.DrawText(float(args[0]), float(args[1]), args[2], ' '.join(args[3:]))
  return True

def ProcessCommand(ia, command):
    words = command.split()
    cmd   = words[0]
    args  = words[1:]
    if cmd == 'color':
      return ProcessColor(ia, args)
    elif cmd == 'font':
      return ProcessFont(ia, args)
    elif cmd == 'rect':
      return ProcessRect(ia, args)
    elif cmd == 'text':
      return ProcessText(ia, args)
    else:
      print(f'*** Unknown command: "{cmd}')
      return False

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
*** Usage: {argv[0]} <input_image> <script> <output_image>

Reads an image file and an annotation script and produces an annotated version
of the image.

An annotation script may contain any number of lines with one of the following:

  [whitespace]            => Blank lines are ignored.
  # ...                   => Comment (ignored).
  color <color>           => Sets the color for text and rectangles.
  font name size          => Sets the font to use for text.
  rect line_width x y w h => Draws a rectangle.
  text x y align <text>   => Draws text. align is one of (left, center, right).
"""

def main():
  if len(argv) != 4:
    print(usage)
    return 1

  input_image  = argv[1]
  script_file  = argv[2]
  output_image = argv[3]

  # Read the input image.
  image = ReadImage(input_image)
  if not image:
    return False

  # Read the script and process the image.
  commands = ReadScript(script_file)
  if not commands:
    return False

  # Process the commands.
  ia = ImageAnnotator(image)
  for command in commands:
    if not ProcessCommand(ia, command):
      return False

  # Write out the resulting image.
  if not ia.Save(output_image):
    print(f'*** Unable to write image to file "{output_image}"')
    return False

  return True

if __name__ == '__main__':
  main()
