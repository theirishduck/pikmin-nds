import bpy

for object in bpy.data.objects:
    object.select = False

bpy.data.objects['red_pikmin'].select = True
bpy.data.objects['Armature'].select = True

bpy.ops.export_scene.fbx(filepath='red_pikmin.fbx', use_selection=True,
    add_leaf_bones=False, path_mode='RELATIVE')