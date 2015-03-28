import os, sys
from PIL import Image
import struct

def toFixed(float_value, fraction=12):
  return int(float_value * pow(2,fraction))

def main(args):
  if not valid_command_line_arguments(args):
        error_exit(1, "Usage: %s <highest point> <file to convert> [file to save]" % args[0])

  input_filename = args[2]
  output_filename = determine_output_filename(input_filename, args)

  source = Image.open(args[2])
  pixels = source.load()
  (width,height) = source.size

  output = bytes()

  output += struct.pack("<II", width, height)

  
  for y in reversed(range(0,height)):
    for x in range(0,width):
      r,g,b = pixels[x,y][:3]
      greyscale_value = min(max(0, int((r + g + b) / 3)), 255)
      world_height = (greyscale_value) * float(args[1]) / 127.0
      output += struct.pack("<I", toFixed(world_height))

  output_file = open(output_filename, "wb")
  output_file.write(output)
  output_file.close()

def valid_command_line_arguments(args):
    return 3 <= len(args) <= 4

def determine_output_filename(input_filename, args):
    filename = substitute_extension(input_filename, ".bin")
    if len(args) >= 4:
        filename = args[3]
    return filename

def substitute_extension(filename, extension):
    return os.path.splitext(filename)[0] + extension

if __name__ == '__main__':
    main(sys.argv)