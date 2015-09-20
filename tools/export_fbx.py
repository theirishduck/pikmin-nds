#!/usr/bin/env python
"""Export a .blend file to an .fbx file.

Usage:
  export_fbx.py --help
  export_fbx.py [--select <object>]* [--output <fbx_file>] <blend_file>

Options:
  --help               Display this help.
  --select <object>    Include named object in export. If none are specified,
                       all objects are exported.
  --output <fbx_file>  The name of the exported .fbx file. If not provided,
                       defaults to the same as the <blend_file> with the
                       ".blend" suffix replaced by ".fbx".
  <blend_file>         The scene from which to export objects.
"""

import os, sys
try:
    import bpy
except ImportError:
    # Not running under Blender; exec this script using Blender instead.
    script_name = os.path.realpath(__file__)
    os.execvp('blender', ['-noaudio', '--background', '--python', script_name,
        '--'] + sys.argv[1:])

ARGUMENT_ERROR = 1
PROCESSING_ERROR = 2

def main():
    args = parse_args(adjust_argv(sys.argv))
    output_filename = (args['--output'] if args['--output'] else
        replace_extension(args['<blend_file>'], '.fbx'))
    export_fbx(args['<blend_file>'], output_filename, select=args['--select'])

def adjust_argv(args):
    return args[args.index('--') + 1:] if '--' in args else []

def parse_args(raw_args):
    if '--help' in raw_args:
        print_usage(sys.stdout, 0)

    args = {
        '<blend_file>': None,
        '--select': [],
        '--output': None
    }
    previous_flag = None
    for arg in raw_args:
        if previous_flag:
            if previous_flag == '--select':
                args['--select'].append(arg)
            elif previous_flag == '--output':
                args['--output'] = arg
            previous_flag = None
            continue

        if arg == '--select':
            previous_flag = '--select'
        elif arg == '--output':
            previous_flag = '--output'
        else:
            if args['<blend_file>']:
                sys.stderr.write('Only one .blend file may be specified.\n\n')
                print_usage(sys.stderr, ARGUMENT_ERROR)
            args['<blend_file>'] = arg

    if not args['<blend_file>']:
        sys.stderr.write('No .blend file specified.\n\n')
        print_usage(sys.stderr, ARGUMENT_ERROR)

    return args

def print_usage(output, exit_code):
    output.write(__doc__)
    sys.exit(exit_code)

def replace_extension(path, extension):
    base_path, _ = os.path.splitext(path)
    return base_path + extension

def export_fbx(blend_filename, fbx_filename, select=None):
    try:
        bpy.ops.wm.open_mainfile(filepath=blend_filename)
    except RuntimeError as error:
        sys.exit(PROCESSING_ERROR)

    if select:
        bpy.ops.object.select_all(action='DESELECT')
        for object in select:
            try:
                bpy.data.objects[object].select = True
            except KeyError:
                sys.stderr.write(
                    'Error: Cannot select object \'%s\': Not found in scene\n' %
                    object)
                sys.exit(PROCESSING_ERROR)
    else:
        bpy.ops.object.select_all(action='SELECT')

    bpy.ops.export_scene.fbx(filepath=fbx_filename, use_selection=True,
        add_leaf_bones=False, path_mode='RELATIVE')

if __name__ == '__main__':
    main()