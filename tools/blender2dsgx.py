#!/usr/local/bin/python
# -*- coding: utf-8 -*-
"""
Blender Export Script - Converts .blend files to .dsgx format.

@author: Nicholas Flynt, Cristián Romo

Usage:
    blender2dsgx.py [options] <blend_file>

Options:
    -h --help       Print this message and exit
    -v --version    Show version number and exit
    --output <fbx_file>  The name of the exported .dsgx file. If not provided,
                         defaults to the same as the <blend_file> with the
                         ".blend" suffix replaced by ".dsgx".
"""

import sys, os
sys.path.append("/opt/dsgx-converter")
sys.path.append("/usr/local/lib/python3.3/site-packages")
from model import dsgx, model
from docopt import docopt
import euclid3 as euclid
import logging
logging.basicConfig(level=logging.DEBUG)
log = logging.getLogger()

try:
    import bpy
except ImportError:
    # Not running under Blender; exec this script using Blender instead.
    script_name = os.path.realpath(__file__)
    os.execvp('blender', ['-noaudio', '--background', '--python', script_name,
        '--'] + sys.argv[1:])

def main():
    arguments = docopt(__doc__, version="0.1", argv=adjust_argv(sys.argv))

    output_filename = (arguments['--output'] if arguments['--output'] else
        replace_extension(arguments['<blend_file>'], '.dsgx'))

    blender_model = import_blendfile(arguments['<blend_file>'])
    display_model_info(blender_model)
    export_dsgx(blender_model, output_filename, False)

def adjust_argv(args):
    return args[args.index('--') + 1:] if '--' in args else []

def replace_extension(path, extension):
    base_path, _ = os.path.splitext(path)
    return base_path + extension

def display_model_info(model):
    log.info("Polygons: %d" % len(model.ActiveMesh().polygons))
    log.info("Vertecies: %d" % len(model.ActiveMesh().vertices))

    log.info("Bounding Sphere: %s" % str(model.bounding_sphere()))
    log.info("Bounding Box: %s" % str(model.bounding_box()))

    log.info("Worst-case Draw Cost (polygons): %d" % model.max_cull_polys())

def import_blendfile(filename):
    print("IMPORT BLENDFILE HERE")
    output_model = model.Model()

    try:
        bpy.ops.wm.open_mainfile(filepath=filename)
    except RuntimeError as error:
        sys.exit(PROCESSING_ERROR)

    for blend_object in bpy.data.objects:
        if blend_object.type == "MESH":
            import_mesh(output_model, blend_object.name, blend_object.data)

    for material in bpy.data.materials:
        print("Adding material: ", material.name)
        scene_ambient = [0.25, 0.25, 0.25]
        ambient = [
            material.ambient * scene_ambient[0],
            material.ambient * scene_ambient[1],
            material.ambient * scene_ambient[2]
        ]
        output_model.addMaterial(material.name, ambient, material.specular_color,
            material.diffuse_color)

    return output_model

def import_mesh(output_model, mesh_name, blender_mesh):
    print("Processing mesh: ", mesh_name)
    output_model.addMesh(mesh_name)
    output_mesh = output_model.ActiveMesh()
    for vertex in blender_mesh.vertices:
        output_mesh.addVertex(euclid.Vector3(
            vertex.co.x,
            vertex.co.z,
            vertex.co.y * -1
        ))
    for polygon in blender_mesh.polygons:
        uvlist = None
        # Here we need to specify normals per-polygon, as opposed
        # to per-vertex.
        normals = [
            blender_mesh.vertices[polygon.vertices[0]].normal,
            blender_mesh.vertices[polygon.vertices[1]].normal,
            blender_mesh.vertices[polygon.vertices[2]].normal
        ]
        # material = None
        material = blender_mesh.materials[polygon.material_index].name
        output_mesh.addPolygon(polygon.vertices, uvlist, normals, material)

def export_dsgx(model, output_filename, vtx10):
    print("EXPORT BLENDFILE HERE")
    dsgx.Writer().write(output_filename, model, vtx10)

if __name__ == '__main__':
    main()
