#!/usr/local/bin/python
# -*- coding: utf-8 -*-
"""
Blender Export Script - Converts .blend files to .dsgx format.

@author: Nicholas Flynt, Cristián Romo

Usage:
    blender2dsgx.py [options] <blend_file>

Options:
    -h --help            Print this message and exit
    -v --version         Show version number and exit
    --vtx10              Output 10-bit vertex coordinates (default is 16-bit)
    --animation=<mode>   Either bone or vertex [default: bone]
    --output <fbx_file>  The name of the exported .dsgx file. If not provided,
                         defaults to the same as the <blend_file> with the
                         ".blend" suffix replaced by ".dsgx".
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
logging.basicConfig(level=logging.WARNING)
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
            replace_extension(arguments['<blend_file>'], '.dsgx'))

        blender_model = import_blendfile(arguments['<blend_file>'], arguments['--animation'])
        display_model_info(blender_model)
        animation_mode = "bone"
        export_dsgx(blender_model, output_filename, arguments['--vtx10'], arguments['--animation'])
    except Exception as e:
        log.error("Something bad happened!")
        traceback.print_exc()
        sys.exit(-1)


def adjust_argv(args):
    return args[args.index('--') + 1:] if '--' in args else []

def replace_extension(path, extension):
    base_path, _ = os.path.splitext(path)
    return base_path + extension

def display_model_info(model):
    for mesh in model.meshes:
        log.info("Mesh: %s", mesh)
        log.info("- Polygons: %d" % len(model.meshes[mesh].polygons))
        log.info("- Vertecies: %d" % len(model.meshes[mesh].vertices))
        log.info("- Bounding Sphere: %s" % str(model.meshes[mesh].bounding_sphere()))
        log.info("- Bounding Box: %s" % str(model.meshes[mesh].bounding_box()))
        log.info("Worst-case Draw Cost (polygons): %d" % model.meshes[mesh].max_cull_polys())

def import_blendfile(filename, animation_mode):
    log.debug("IMPORT BLENDFILE HERE")
    output_model = model.Model()
    output_model.global_matrix = blend_matrix_to_euclid(blender_conversion_matrix())

    try:
        bpy.ops.wm.open_mainfile(filepath=filename)
    except RuntimeError as error:
        log.error("Couldn't open " + filename + ", bailing.")
        sys.exit(PROCESSING_ERROR)

    for blend_object in bpy.data.objects:
        if blend_object.hide_render == False:
            if blend_object.type == "MESH":
                import_mesh(output_model, blend_object.name, blend_object, animation_mode == "bone")

    for material in bpy.data.materials:
        import_material(output_model, material.name, material)

    import_animations(output_model)

    return output_model

def import_material(output_model, material_name, blender_material):
    log.info("Adding material: ", material_name)
    scene_ambient = bpy.data.worlds[0].ambient_color
    ambient = [
        blender_material.ambient * scene_ambient[0],
        blender_material.ambient * scene_ambient[1],
        blender_material.ambient * scene_ambient[2]
    ]
    diffuse = blender_material.diffuse_color * blender_material.diffuse_intensity
    specular = blender_material.specular_color * blender_material.specular_intensity
    emit = blender_material.diffuse_color * blender_material.emit

    texture = None
    texture_width = 0
    texture_height = 0
    blender_texture = blender_material.active_texture
    if blender_texture != None:
        if blender_texture.type == "IMAGE":
            texture = blender_texture.image.filepath
            texture = os.path.basename(texture)
            texture = os.path.splitext(texture)[0]
            texture_width = blender_texture.image.size[0]
            texture_height = blender_texture.image.size[1]

    output_model.addMaterial(material_name, ambient, specular, diffuse, emit,
            texture, texture_width, texture_height)

def import_mesh(output_model, mesh_name, blender_object, use_vertex_groups=True):
    blender_mesh = blender_object.data
    log.info("Processing mesh: ", mesh_name)
    output_mesh = output_model.addMesh(mesh_name)
    for vertex in blender_mesh.vertices:
        group = "group"
        if use_vertex_groups and len(vertex.groups) == 1:
            group = blender_object.vertex_groups[vertex.groups[0].group].name
        position = euclid.Vector3(vertex.co.x, vertex.co.y, vertex.co.z)
        output_mesh.addVertex(position, group)

    for polygon in blender_mesh.polygons:
        uvlist = None
        if len(blender_mesh.uv_layers) > 0:
            uv_data = blender_mesh.uv_layers[0].data
            uvlist = [uv_data[polygon.loop_start + i].uv for i in range(0, polygon.loop_total)]
        # Here we need to specify normals per-polygon, as opposed
        # to per-vertex.
        normals = [blender_mesh.vertices[vertex].normal.normalized() * 0.95 for vertex in polygon.vertices]
        material = blender_mesh.materials[polygon.material_index].name
        smooth_shading = polygon.use_smooth;
        output_mesh.addPolygon(polygon.vertices, uvlist, normals, material, smooth_shading)

def blender_conversion_matrix():
    m = mathutils.Matrix().to_4x4()
    m.identity()
    m[1][1] = 0
    m[1][2] = 1
    m[2][1] = -1
    m[2][2] = 0
    return m

def import_vertex_animations(output_model, mesh_name, blender_object):
    armature = bpy.data.objects["Armature"]
    actions = bpy.data.actions
    log.debug("Importing VERTEX/NORMAL animations...")

    for action in actions:
        if action.id_root == "ARMATURE" or action.id_root == "OBJECT":
            log.debug("Switching to action: ", action.name)
            armature.animation_data.action = action

            # Initialize an empty list of channels, one for each vertex
            mesh = blender_object.to_mesh(bpy.data.scenes[0], True, "PREVIEW")
            vertex_channels = {i: [] for i in range(len(mesh.vertices))}
            normal_channels = {i: [] for i in range(len(mesh.vertices))}

            frame_start = action.frame_range[0]
            frame_end = action.frame_range[1]
            for frame in range(int(action.frame_range[0]), int(action.frame_range[1]), 2):
                bpy.data.scenes[0].frame_set(frame)
                bpy.data.scenes[0].update()

                mesh = blender_object.to_mesh(bpy.data.scenes[0], True, "PREVIEW")
                for vertex_index in range(len(mesh.vertices)):
                    vertex = mesh.vertices[vertex_index]
                    vertex_channels[vertex_index].append(vertex.co)
                    normal_channels[vertex_index].append(vertex.normal)

            animation_length = int(math.floor((action.frame_range[1] - action.frame_range[0]) / 2))
            vertex_animation = output_model.create_animation("Armature|" + action.name, "vertex", mesh_name)
            vertex_animation.mesh = mesh_name
            vertex_animation.length = animation_length
            for vertex_index, vertex_positions in vertex_channels.items():
                vertex_animation.add_channel(vertex_index, vertex_positions)

            normal_animation = output_model.create_animation("Armature|" + action.name, "normal", mesh_name)
            normal_animation.length = animation_length
            normal_animation.mesh = mesh_name
            for vertex_index, normal_directions in normal_channels.items():
                normal_animation.add_channel(vertex_index, normal_directions)
            log.info("Created vertex animation ", action.name, " with ", animation_length, " frames.")


def import_bone_animations(output_model):
    # Note: I HATE that this is hardcoded here, but I can't find another way
    # to do this. Future self, save me!
    armature = bpy.data.objects["Armature"]
    actions = bpy.data.actions

    log.debug("Importing BONE animations...")

    for action in actions:
        if action.id_root == "ARMATURE" or action.id_root == "OBJECT":
            log.debug("Switching to action: ", action.name)
            armature.animation_data.action = action

            # Initialize an empty list of channels, based on the bones contained
            # in the armature
            bone_channels = {bone.name: [] for bone in armature.data.bones}

            # Note: step is 2, because blender defaults to 60 FPS, and we want
            # to export at 30 FPS.
            for frame in range(int(action.frame_range[0]), int(action.frame_range[1]), 2):
                bpy.data.scenes[0].frame_set(frame)
                # Write out the bone transformation matrix for each bone
                for posebone in armature.pose.bones:
                    bind_pose = posebone.bone.matrix_local
                    object_space_transform = posebone.matrix
                    # final_transform = blender_conversion_matrix() * object_space_transform * bind_pose.inverted()
                    final_transform = object_space_transform * bind_pose.inverted()
                    bone_channels[posebone.bone.name].append(blend_matrix_to_euclid(final_transform))
                # Write out the transformed vertices and normals

            animation = output_model.create_animation("Armature|" + action.name, "bone")
            animation.length = int(math.floor((action.frame_range[1] - action.frame_range[0]) / 2))
            for bone_name, bone_transforms in bone_channels.items():
                animation.add_channel(bone_name, bone_transforms)
            log.info("Created animation ", action.name, " with ", animation.length, " frames.")

def import_animations(output_model):
    # first, grab the armature itself. Bail if there is no armature.
    if len(bpy.data.armatures) == 0:
        log.info("No armature! Skipping animation processing.")
        return
    if len(bpy.data.armatures) > 1:
        log.warning("More than one armature! This is unsupported; bailing.")
        return

    import_bone_animations(output_model)

    for blend_object in bpy.data.objects:
        if blend_object.hide_render == False:
            if blend_object.type == "MESH":
                import_vertex_animations(output_model, blend_object.name, blend_object)

def blend_matrix_to_euclid(matrix):
    return euclid.Matrix4.new(
        matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3],
        matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3],
        matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3],
        matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3])

def export_dsgx(model, output_filename, vtx10, animation_mode):
    log.debug("EXPORT BLENDFILE HERE")
    dsgx.Writer().write(output_filename, model, vtx10, animation_mode)

if __name__ == '__main__':
    main()
