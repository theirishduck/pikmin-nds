#!/usr/local/bin/python
# -*- coding: utf-8 -*-
"""
Blender Level Export Script - Given a .blend file, creates a Level file with
  the embedded scene geometry and positioning information.

@author: Nicholas Flynt, Cristián Romo

Usage:
    blender2dsgx.py [options] <blend_file>

Options:
    -h --help            Print this message and exit
    -v --version         Show version number and exit
    --output <level_file>  The name of the exported .dsgx file. If not provided,
                         defaults to the same as the <blend_file> with the
                         ".blend" suffix replaced by ".level".
"""

import sys, os, logging, traceback, math
sys.path.append("/opt/dsgx-converter")
sys.path.append("/usr/local/lib/python3.4/dist-packages")
try:
    from model import dsgx, model
    from docopt import docopt
    import euclid3 as euclid
except Exception as e:
    traceback.print_exc()
    sys.exit(-1)
logging.basicConfig(level=logging.DEBUG)
log = logging.getLogger()

try:
    import bpy
except ImportError:
    # Not running under Blender; exec this script using Blender instead.
    script_name = os.path.realpath(__file__)
    os.execvp('blender', ['-noaudio', '--background', '--python', script_name,
        '--'] + sys.argv[1:])

import mathutils

def main():
    try:
        arguments = docopt(__doc__, version="0.1", argv=adjust_argv(sys.argv))
        output_filename = (arguments['--output'] if arguments['--output'] else
            replace_extension(arguments['<blend_file>'], '.level'))
        commands = spawn_commands_from_blendfile(arguments['<blend_file>'])
        write_level(commands, output_filename)
        
    except Exception as e:
        log.error("Something bad happened!")
        traceback.print_exc()
        sys.exit(-1)


def adjust_argv(args):
    return args[args.index('--') + 1:] if '--' in args else []

def replace_extension(path, extension):
    base_path, _ = os.path.splitext(path)
    return base_path + extension

def open_blendfile(filename):
    log.debug("OPEN BLENDFILE HERE")
    try:
        bpy.ops.wm.open_mainfile(filepath=filename)
    except RuntimeError as error:
        log.error("Couldn't open " + filename + ", bailing.")
        sys.exit(PROCESSING_ERROR)

def level_command(command_string, *args):
    return command_string + " " + " ".join(map(str, args))
    
def write_level(commands, output_filename):
    with open(output_filename, "w") as file:
        file.write("\n".join(commands))

def opengl_pos_from_blender(position):
    return [position.x, position.z, position.y * -1]

def object_position(blend_object):
    pos = ["%f" % coordinate for coordinate in opengl_pos_from_blender(blend_object.location)]
    return level_command("position", pos[0], pos[1], pos[2])

def static_mesh(level_name, blend_object):
    commands = []
    commands.append(level_command("spawn", "Static"))
    commands.append(level_command("actor", level_name))
    commands.append(level_command("mesh", blend_object.name))

    commands.append(object_position(blend_object))
    return commands

def spawn_object(blend_object):
    commands = []
    commands.append(level_command("spawn", blend_object["spawn"]))
    commands.append(object_position(blend_object))
    return commands

def spawn_commands_from_blendfile(filename):
    open_blendfile(filename)
    level_name, _ = os.path.splitext(os.path.splitext(os.path.basename(filename))[0])
    log.debug("File Name: " + filename)
    log.debug("Level Name: " + level_name)

    commands = []

    # For now: assume a heightmap exists which matches the level name
    commands.append(level_command("heightmap", level_name))

    for blend_object in bpy.data.objects:
        if blend_object.type == "MESH":
            if blend_object.hide_render == False:
                commands.extend(static_mesh(level_name, blend_object))
        if blend_object.type == "EMPTY":
            if "spawn" in blend_object:
                commands.extend(spawn_object(blend_object))

    return commands

def blender_conversion_matrix():
    m = mathutils.Matrix().to_4x4()
    m.identity()
    m[1][1] = 0
    m[1][2] = 1
    m[2][1] = -1
    m[2][2] = 0
    return m

def blend_matrix_to_euclid(matrix):
    return euclid.Matrix4.new(
        matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3],
        matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3],
        matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3],
        matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3])

if __name__ == '__main__':
    main()
