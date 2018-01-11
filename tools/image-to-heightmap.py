import os, sys
from PIL import Image
import struct

def toFixed(float_value, fraction=12):
  return int(float_value * pow(2,fraction))

def main(args):
  if not valid_command_line_arguments(args):
        sys.exit("Usage: %s <file to convert> [file to save]" % args[0])

  input_filename = args[1]
  output_filename = determine_output_filename(input_filename, args)

  source = Image.open(input_filename)
  pixels = source.load()
  (width,height) = source.size

  output = bytes()

  output += struct.pack("<II", width, height)

  for y in range(0,height):
    for x in range(0,width):
      r,g,b = pixels[x,y][:3]
      # Note: Blender exports heightmaps normalized to 0-127 for whatever reason, instead of
      # from 0-255 as one might expect. This is why our adjusted max here is 127, instead of 255.
      greyscale_value = min(max(0, int((r + g + b) / 3)), 127)
      output += struct.pack("<B", greyscale_value)

  output_file = open(output_filename, "wb")
  output_file.write(output)
  output_file.close()

def valid_command_line_arguments(args):
    return 2 <= len(args) <= 3

def determine_output_filename(input_filename, args):
    filename = substitute_extension(input_filename, ".height")
    if len(args) >= 3:
        filename = args[2]
    return filename

def substitute_extension(filename, extension):
    return os.path.splitext(filename)[0] + extension

if __name__ == '__main__':
    main(sys.argv)