"""PokoX character generator (Blender 5.x, runs inside Blender).

Builds the blocky R6-style PokoX character as a game-ready, skinned asset:
six separate body meshes, a 7-bone rig, ten attachment empties, a UV-mapped
face baked onto the head's front face, matte PBR materials, and exports.

Proportions are measured off the art reference rather than guessed: the
reference drawing is 468 px tall, and every part below is a pixel measurement
of it converted at 1.80 m / 468 px. That is why the torso is nearly square and
the limbs are thick -- the earlier spec numbers produced a tall, slim figure
that did not read like the reference at all.

Usage from the Blender MCP `execute_blender_code` tool:

    ns = {"__name__": "pokox_character", "__file__": SCRIPT}
    exec(compile(open(SCRIPT, encoding="utf-8").read(), SCRIPT, "exec"), ns)
    ns["build_character"]()

Nothing here is destructive: each build creates a fresh scene, and the .blend
is written as a copy so the user's current file is never replaced.
"""

from __future__ import annotations

import json
import math
import os

import bmesh
import bpy
import numpy as np
from mathutils import Matrix, Vector

# --------------------------------------------------------------------------- #
# colour
# --------------------------------------------------------------------------- #


def _srgb_to_linear(value: float) -> float:
    value /= 255.0
    return value / 12.92 if value <= 0.04045 else ((value + 0.055) / 1.055) ** 2.4


def rgb(r, g, b):
    """Spec colours are sRGB 0-255; Principled wants linear."""
    return (_srgb_to_linear(r), _srgb_to_linear(g), _srgb_to_linear(b), 1.0)


YELLOW = rgb(245, 205, 48)
RED = rgb(220, 50, 50)
BLUE = rgb(50, 100, 220)
INK = rgb(22, 22, 24)

# --------------------------------------------------------------------------- #
# proportions -- measured from the reference render, in metres
# --------------------------------------------------------------------------- #

TARGET_HEIGHT = 1.80
PX = TARGET_HEIGHT / 468.0            # reference is 468 px from sole to crown

TORSO_W = 178 * PX                    # 0.685
TORSO_H = 184 * PX                    # 0.708  (nearly square, like the art)
TORSO_D = TORSO_W * 0.50              # R6 torso is half as deep as it is wide
HEAD_W = 105 * PX                     # 0.404  (reduced for better proportion)
HEAD_H = 105 * PX                     # 0.404  (cylindrical, Roblox R6 style)
HEAD_D = HEAD_W
ARM_W = 88 * PX                       # 0.339
ARM_D = ARM_W
LEG_W = TORSO_W * 0.50                # two legs together fill the torso width
LEG_D = LEG_W
LEG_H = TARGET_HEIGHT - TORSO_H - HEAD_H

LEG_BOTTOM = 0.0
LEG_TOP = LEG_H
TORSO_BOTTOM = LEG_TOP
TORSO_TOP = TORSO_BOTTOM + TORSO_H
HEAD_BOTTOM = TORSO_TOP
HEAD_TOP = HEAD_BOTTOM + HEAD_H

# The reference shows the arms flush with the shoulder AND the hip: each arm is
# exactly as long as the torso.
ARM_TOP = TORSO_TOP
ARM_BOTTOM = TORSO_BOTTOM
ARM_LENGTH = TORSO_H
ARM_MID = (ARM_TOP + ARM_BOTTOM) / 2
ARM_X = TORSO_W / 2 + ARM_W / 2
LEG_X = LEG_W / 2
LEG_MID = (LEG_TOP + LEG_BOTTOM) / 2

# One edge radius everywhere is what makes the reference read as a soft toy
# instead of six hard boxes (measured: ~18 px of the 120 px head).
EDGE_R = 0.068
BEVEL_SEGMENTS = 8
HEAD_BEVEL_SEGMENTS = 12

# Studio presentation. The reference is a filmic render: under Filmic the
# primaries land on the art's values instead of on raw albedo.
VIEW_TRANSFORM = "Filmic"
EXPOSURE = 0.6
BACKDROP_EMISSION = 0.0196

HEAD_FLAT_W = HEAD_W - 2 * EDGE_R
HEAD_FLAT_H = HEAD_H - 2 * EDGE_R

# Face marks, measured off the reference head (120 x 117 px) and shifted 4 mm
# up so the smile's lowest point clears the flat face's bottom tangent.
FACE_LIFT = 0.004
EYE_X = 0.0499
EYE_RX, EYE_RZ = 0.0154, 0.0327
EYE_Z = 0.300 + FACE_LIFT - EDGE_R          # from the flat face's bottom edge
MOUTH_R = 0.0697
MOUTH_STROKE = 0.0115
MOUTH_ZC = 0.0903
MOUTH_ARC = math.radians(166.6)

BONE_SPEC = [
    # name,        head,                  tail,                    parent, deform
    ("Root",       (0.0, 0.0, 0.0),       (0.0, 0.0, 0.14),        None,         False),
    ("Hip",        (0.0, 0.0, LEG_TOP),   (0.0, 0.0, TORSO_TOP),   "Root",       True),
    ("Neck",       (0.0, 0.0, TORSO_TOP), (0.0, 0.0, HEAD_TOP - 0.05), "Hip",    True),
    ("Shoulder.L", (ARM_X, 0.0, ARM_TOP), (ARM_X, 0.0, ARM_BOTTOM), "Hip",       True),
    ("Shoulder.R", (-ARM_X, 0.0, ARM_TOP), (-ARM_X, 0.0, ARM_BOTTOM), "Hip",     True),
    ("Hip.L",      (LEG_X, 0.0, LEG_TOP), (LEG_X, 0.0, LEG_BOTTOM), "Hip",       True),
    ("Hip.R",      (-LEG_X, 0.0, LEG_TOP), (-LEG_X, 0.0, LEG_BOTTOM), "Hip",     True),
]

# attachment name -> (position, bone it rides on)
# Positions are on the surface of each body part
ATTACHMENTS = {
    # The character faces -Y (Blender front view, glTF +Z forward after export).
    # Hand attachments at bottom of arms
    "ATT_LeftHand":      ((ARM_X, -ARM_D / 2, ARM_BOTTOM + 0.06), "Shoulder.L"),
    "ATT_RightHand":     ((-ARM_X, -ARM_D / 2, ARM_BOTTOM + 0.06), "Shoulder.R"),
    "ATT_LeftHandBack":  ((ARM_X, ARM_D / 2, ARM_BOTTOM + 0.06), "Shoulder.L"),
    "ATT_RightHandBack": ((-ARM_X, ARM_D / 2, ARM_BOTTOM + 0.06), "Shoulder.R"),
    # Shoulder attachments
    "ATT_LeftShoulder":  ((ARM_X + 0.05, 0, ARM_TOP - 0.05), "Shoulder.L"),
    "ATT_RightShoulder": ((-ARM_X - 0.05, 0, ARM_TOP - 0.05), "Shoulder.R"),
    "ATT_LeftShoulderSide": ((ARM_X + 0.08, -ARM_D / 2, ARM_TOP - 0.1), "Shoulder.L"),
    "ATT_RightShoulderSide": ((-ARM_X - 0.08, -ARM_D / 2, ARM_TOP - 0.1), "Shoulder.R"),
    # Elbow attachments
    "ATT_LeftElbow":     ((ARM_X, 0, ARM_MID), "Shoulder.L"),
    "ATT_RightElbow":    ((-ARM_X, 0, ARM_MID), "Shoulder.R"),
    "ATT_LeftElbowSide": ((ARM_X + 0.05, -ARM_D / 2, ARM_MID), "Shoulder.L"),
    "ATT_RightElbowSide": ((-ARM_X - 0.05, -ARM_D / 2, ARM_MID), "Shoulder.R"),
    # Upper arm attachments
    "ATT_LeftUpperArm":  ((ARM_X, 0, ARM_TOP - ARM_LENGTH * 0.25), "Shoulder.L"),
    "ATT_RightUpperArm": ((-ARM_X, 0, ARM_TOP - ARM_LENGTH * 0.25), "Shoulder.R"),
    # Foot attachments at bottom of legs
    "ATT_LeftFoot":      ((LEG_X, -LEG_D / 2, LEG_BOTTOM + 0.06), "Hip.L"),
    "ATT_RightFoot":     ((-LEG_X, -LEG_D / 2, LEG_BOTTOM + 0.06), "Hip.R"),
    "ATT_LeftFootBack":  ((LEG_X, LEG_D / 2, LEG_BOTTOM + 0.06), "Hip.L"),
    "ATT_RightFootBack": ((-LEG_X, LEG_D / 2, LEG_BOTTOM + 0.06), "Hip.R"),
    "ATT_LeftFootSide":  ((LEG_X + 0.05, -LEG_D / 2, LEG_BOTTOM + 0.08), "Hip.L"),
    "ATT_RightFootSide": ((-LEG_X - 0.05, -LEG_D / 2, LEG_BOTTOM + 0.08), "Hip.R"),
    # Hip/upper leg attachments
    "ATT_LeftHip":       ((LEG_X + 0.05, 0, LEG_TOP - 0.05), "Hip.L"),
    "ATT_RightHip":      ((-LEG_X - 0.05, 0, LEG_TOP - 0.05), "Hip.R"),
    "ATT_LeftHipSide":   ((LEG_X + 0.08, -LEG_D / 2, LEG_TOP - 0.1), "Hip.L"),
    "ATT_RightHipSide":  ((-LEG_X - 0.08, -LEG_D / 2, LEG_TOP - 0.1), "Hip.R"),
    # Knee attachments
    "ATT_LeftKnee":      ((LEG_X, 0, LEG_MID), "Hip.L"),
    "ATT_RightKnee":     ((-LEG_X, 0, LEG_MID), "Hip.R"),
    "ATT_LeftKneeSide":  ((LEG_X + 0.05, -LEG_D / 2, LEG_MID), "Hip.L"),
    "ATT_RightKneeSide": ((-LEG_X - 0.05, -LEG_D / 2, LEG_MID), "Hip.R"),
    # Thigh attachments
    "ATT_LeftThigh":     ((LEG_X, 0, LEG_TOP - LEG_H * 0.25), "Hip.L"),
    "ATT_RightThigh":    ((-LEG_X, 0, LEG_TOP - LEG_H * 0.25), "Hip.R"),
    # Head attachments
    "ATT_HeadTop":       ((0.0, 0.0, HEAD_TOP), "Neck"),
    "ATT_HeadFront":     ((0.0, -HEAD_D / 2, HEAD_BOTTOM + HEAD_H * 0.55), "Neck"),
    "ATT_HeadBack":      ((0.0, HEAD_D / 2, HEAD_BOTTOM + HEAD_H * 0.55), "Neck"),
    "ATT_HeadLeft":      ((HEAD_D / 2, 0, HEAD_BOTTOM + HEAD_H * 0.55), "Neck"),
    "ATT_HeadRight":     ((-HEAD_D / 2, 0, HEAD_BOTTOM + HEAD_H * 0.55), "Neck"),
    "ATT_HeadEarLeft":   ((HEAD_D / 2 - 0.02, 0, HEAD_BOTTOM + HEAD_H * 0.65), "Neck"),
    "ATT_HeadEarRight":  ((-HEAD_D / 2 + 0.02, 0, HEAD_BOTTOM + HEAD_H * 0.65), "Neck"),
    "ATT_Face":          ((0.0, -HEAD_D / 2 + 0.002, HEAD_BOTTOM + HEAD_H * 0.55), "Neck"),
    # Body attachments on torso surface
    "ATT_Chest":         ((0.0, -TORSO_D / 2, TORSO_BOTTOM + TORSO_H * 0.70), "Hip"),
    "ATT_Back":          ((0.0, TORSO_D / 2, TORSO_BOTTOM + TORSO_H * 0.70), "Hip"),
    "ATT_LeftChest":     ((TORSO_W / 2 - 0.05, -TORSO_D / 2, TORSO_BOTTOM + TORSO_H * 0.70), "Hip"),
    "ATT_RightChest":    ((-TORSO_W / 2 + 0.05, -TORSO_D / 2, TORSO_BOTTOM + TORSO_H * 0.70), "Hip"),
    "ATT_LeftBack":      ((TORSO_W / 2 - 0.05, TORSO_D / 2, TORSO_BOTTOM + TORSO_H * 0.70), "Hip"),
    "ATT_RightBack":     ((-TORSO_W / 2 + 0.05, TORSO_D / 2, TORSO_BOTTOM + TORSO_H * 0.70), "Hip"),
    # Belt attachments
    "ATT_Belt":          ((0.0, 0.0, LEG_TOP + 0.015), "Hip"),
    "ATT_LeftBelt":      ((TORSO_W / 2 - 0.05, 0, LEG_TOP + 0.015), "Hip"),
    "ATT_RightBelt":     ((-TORSO_W / 2 + 0.05, 0, LEG_TOP + 0.015), "Hip"),
    "ATT_BeltFront":     ((0.0, -TORSO_D / 2, LEG_TOP + 0.015), "Hip"),
    "ATT_BeltBack":      ((0.0, TORSO_D / 2, LEG_TOP + 0.015), "Hip"),
    # Shoulder bag attachments
    "ATT_LeftShoulderBag": ((ARM_X + 0.1, -ARM_D / 2, ARM_TOP - 0.15), "Shoulder.L"),
    "ATT_RightShoulderBag": ((-ARM_X - 0.1, -ARM_D / 2, ARM_TOP - 0.15), "Shoulder.R"),
    # Backpack attachment
    "ATT_Backpack":      ((0.0, TORSO_D / 2 + 0.05, TORSO_BOTTOM + TORSO_H * 0.80), "Hip"),
    "ATT_BackpackTop":   ((0.0, TORSO_D / 2 + 0.05, TORSO_BOTTOM + TORSO_H * 0.90), "Hip"),
    # Neck attachment
    "ATT_Neck":          ((0.0, 0, HEAD_BOTTOM - 0.02), "Neck"),
    "ATT_NeckFront":     ((0.0, -HEAD_D / 2 - 0.02, HEAD_BOTTOM - 0.02), "Neck"),
    "ATT_NeckBack":      ((0.0, HEAD_D / 2 + 0.02, HEAD_BOTTOM - 0.02), "Neck"),
}


def p(x, y, z):
    """World point (everything is authored in metres)."""
    return Vector((x, y, z))


def m(value):
    """World length (everything is authored in metres)."""
    return value


# --------------------------------------------------------------------------- #
# small helpers
# --------------------------------------------------------------------------- #


def _link(obj, collection):
    for coll in list(obj.users_collection):
        coll.objects.unlink(obj)
    collection.objects.link(obj)
    return obj


def _material(name, color, roughness=0.8, metallic=0.0, specular=0.1):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    bsdf = next((n for n in mat.node_tree.nodes if n.type == "BSDF_PRINCIPLED"), None)
    if bsdf is not None:
        bsdf.inputs["Base Color"].default_value = color
        bsdf.inputs["Roughness"].default_value = roughness
        bsdf.inputs["Metallic"].default_value = metallic
        if "Specular IOR Level" in bsdf.inputs:
            bsdf.inputs["Specular IOR Level"].default_value = specular
    mat.diffuse_color = color
    mat.roughness = roughness
    mat.metallic = metallic
    return mat


def _uv_per_face(bm, scale):
    """Box projection: one planar UV island per face, normalised to one tile."""
    layer = bm.loops.layers.uv.new("UVMap")
    for face in bm.faces:
        normal = face.normal
        axis = max(range(3), key=lambda i: abs(normal[i]))
        for loop in face.loops:
            co = loop.vert.co
            if axis == 0:
                uv = (co.y, co.z)
            elif axis == 1:
                uv = (co.x, co.z)
            else:
                uv = (co.x, co.y)
            loop[layer].uv = (
                (uv[0] / scale) + 0.5,
                (uv[1] / scale) + 0.5,
            )


def _finish_mesh(name, bm, collection, material, shade="flat"):
    mesh = bpy.data.meshes.new(name + "_mesh")
    bm.normal_update()
    bm.to_mesh(mesh)
    bm.free()
    obj = bpy.data.objects.new(name, mesh)
    collection.objects.link(obj)
    if material is not None:
        mesh.materials.append(material)
    if shade == "smooth":
        for poly in mesh.polygons:
            poly.use_smooth = True
    return obj


def _crease_flat_faces(mesh):
    """Keep the six flats flat and let only the roundover shade smoothly.

    A beveled box shaded fully smooth turns every flat face into a pillow; the
    harden-normals option does not survive here, so the tangent rings are
    creased explicitly and the mesh is shaded smooth afterwards.
    """
    flats = [
        poly
        for poly in mesh.polygons
        if max(abs(poly.normal.x), abs(poly.normal.y), abs(poly.normal.z)) > 0.999
        and poly.area > 0.008
    ]
    keys = set()
    for poly in flats:
        keys.update(poly.edge_keys)
    index = {edge.key: edge.index for edge in mesh.edges}
    for key in keys:
        edge_index = index.get(key)
        if edge_index is not None:
            mesh.edges[edge_index].use_edge_sharp = True


# --------------------------------------------------------------------------- #
# geometry
# --------------------------------------------------------------------------- #


def add_rounded_box(name, size, center, collection, material,
                    radius=EDGE_R, segments=BEVEL_SEGMENTS):
    """Box with one uniform, toy-soft roundover; origin at the part centre."""
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    for vert in bm.verts:
        vert.co.x *= size[0]
        vert.co.y *= size[1]
        vert.co.z *= size[2]
    _uv_per_face(bm, max(size))
    mesh = bpy.data.meshes.new(name + "_mesh")
    bm.to_mesh(mesh)
    bm.free()

    obj = bpy.data.objects.new(name, mesh)
    collection.objects.link(obj)
    if material is not None:
        mesh.materials.append(material)
    obj.location = Vector(center)

    modifier = obj.modifiers.new("Roundover", "BEVEL")
    modifier.width = radius
    modifier.segments = segments
    modifier.limit_method = "ANGLE"
    modifier.angle_limit = math.radians(30.0)
    modifier.use_clamp_overlap = True

    for other in bpy.context.selected_objects:
        other.select_set(False)
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.modifier_apply(modifier=modifier.name)
    obj.select_set(False)

    _crease_flat_faces(mesh)
    for poly in mesh.polygons:
        poly.use_smooth = True
    return obj


def _revolve(profile, segments=48, scale_fn=None):
    """Surface of revolution around Z from (radius, z) profile points."""
    bm = bmesh.new()
    rows = []
    for radius_i, z in profile:
        if radius_i <= 1e-6:
            rows.append([bm.verts.new(scale_fn(0.0, 0.0, z))])
            continue
        ring = []
        for index in range(segments):
            angle = 2 * math.pi * index / segments
            ring.append(
                bm.verts.new(
                    scale_fn(radius_i * math.cos(angle), radius_i * math.sin(angle), z)
                )
            )
        rows.append(ring)

    for lower, upper in zip(rows, rows[1:]):
        if len(lower) == 1 and len(upper) == 1:
            continue
        if len(lower) == 1:
            pole = lower[0]
            for index in range(segments):
                bm.faces.new((pole, upper[index], upper[(index + 1) % segments]))
        elif len(upper) == 1:
            pole = upper[0]
            for index in range(segments):
                bm.faces.new((lower[index], lower[(index + 1) % segments], pole))
        else:
            for index in range(segments):
                nxt = (index + 1) % segments
                bm.faces.new((lower[index], lower[nxt], upper[nxt], upper[index]))

    bmesh.ops.recalc_face_normals(bm, faces=list(bm.faces))
    return bm


def add_head(name, collection, material, face_material, texture,
             segments=HEAD_BEVEL_SEGMENTS):
    """Cylindrical head with beveled edges (Roblox R6 style); the face texture lives on its front face."""
    # Use higher segment count for smooth cylinder
    smooth_segments = 32  # High segment count for smooth appearance

    bm = bmesh.new()
    radius = HEAD_W / 2

    # Create vertices for cylinder manually
    verts = []
    rings = 2  # Bottom and top rings

    for ring_idx in range(rings):
        z = ring_idx * HEAD_H if ring_idx == 1 else 0.0
        ring_verts = []
        for i in range(smooth_segments):
            angle = 2 * math.pi * i / smooth_segments
            x = radius * math.cos(angle)
            y = radius * math.sin(angle)
            ring_verts.append(bm.verts.new((x, y, z)))
        verts.append(ring_verts)

    # Create side faces
    for i in range(smooth_segments):
        next_i = (i + 1) % smooth_segments
        bm.faces.new((verts[0][i], verts[0][next_i], verts[1][next_i], verts[1][i]))

    # Create top cap
    top_center = bm.verts.new((0, 0, HEAD_H))
    for i in range(smooth_segments):
        next_i = (i + 1) % smooth_segments
        bm.faces.new((verts[1][i], verts[1][next_i], top_center))

    # Create bottom cap
    bottom_center = bm.verts.new((0, 0, 0))
    for i in range(smooth_segments):
        next_i = (i + 1) % smooth_segments
        bm.faces.new((bottom_center, verts[0][next_i], verts[0][i]))

    bmesh.ops.recalc_face_normals(bm, faces=list(bm.faces))

    # Create mesh and object
    mesh = bpy.data.meshes.new(name + "_mesh")
    bm.to_mesh(mesh)
    bm.free()

    obj = bpy.data.objects.new(name, mesh)
    collection.objects.link(obj)
    # Position head at the neck bone location (TORSO_TOP)
    obj.location = Vector((0.0, 0.0, TORSO_TOP))

    # Add materials
    mesh.materials.append(material)
    mesh.materials.append(face_material)

    # Apply bevel modifier to soften edges (Roblox R6 style)
    modifier = obj.modifiers.new("HeadBevel", "BEVEL")
    modifier.width = EDGE_R
    modifier.segments = segments
    modifier.limit_method = "ANGLE"
    modifier.angle_limit = math.radians(30.0)
    modifier.use_clamp_overlap = True

    # Apply the modifier
    for other in bpy.context.selected_objects:
        other.select_set(False)
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.modifier_apply(modifier=modifier.name)
    obj.select_set(False)

    # Create UV layer
    uv_layer = mesh.uv_layers.new(name="UVMap")
    
    # UV map the entire head using simple cylindrical projection
    # The face will be handled separately via ATT_Face attachment point
    for poly in mesh.polygons:
        for loop_index in poly.loop_indices:
            co = mesh.vertices[mesh.loops[loop_index].vertex_index].co
            # Simple cylindrical UV mapping for the head
            angle = math.atan2(co.y, co.x)
            u = (angle / (2 * math.pi)) + 0.5
            v = (co.z / HEAD_H) + 0.5
            uv_layer.data[loop_index].uv = (u, v)
    
    # Assign all polygons to yellow material (face will be separate attachment)
    for poly in mesh.polygons:
        poly.material_index = 0
    
    # Smooth shading for the cylinder
    for poly in mesh.polygons:
        poly.use_smooth = True

    # Connect texture to face material (for ATT_Face attachment)
    tree = face_material.node_tree
    image_node = tree.nodes.new("ShaderNodeTexImage")
    image_node.image = texture
    image_node.interpolation = "Cubic"
    image_node.location = (-420, 220)
    bsdf = next(n for n in tree.nodes if n.type == "BSDF_PRINCIPLED")
    tree.links.new(image_node.outputs["Color"], bsdf.inputs["Base Color"])
    
    return obj


def create_conforming_face_mesh(name, collection, material, texture):
    """Create a face mesh that conforms to the cylinder curvature."""
    bm = bmesh.new()
    
    # Create a curved face mesh that wraps around the cylinder
    # This will be a section of the cylinder facing -Y direction
    # Smaller size to match Roblox R6 proportions and avoid stretching
    face_width = HEAD_W * 0.5  # 50% of head width
    face_height = HEAD_H * 0.4  # 40% of head height (smaller to avoid stretching)
    segments_x = 16  # Higher resolution for better curvature
    segments_z = 8  # Reduced height resolution
    radius = HEAD_D / 2
    
    # Generate vertices for a curved patch on the cylinder front
    verts = []
    for iz in range(segments_z + 1):
        for ix in range(segments_x + 1):
            # Calculate position on cylinder surface
            x = -face_width / 2 + (ix / segments_x) * face_width
            z = -face_height / 2 + (iz / segments_z) * face_height
            
            # Calculate y position on cylinder surface (front, facing -Y)
            # The cylinder equation: x^2 + y^2 = radius^2
            # For front face: y = -sqrt(radius^2 - x^2)
            y_sq = radius * radius - x * x
            if y_sq > 0:
                y = -math.sqrt(y_sq)
            else:
                y = -radius  # Edge case
            
            verts.append(bm.verts.new((x, y, z)))
    
    # Create faces
    for iz in range(segments_z):
        for ix in range(segments_x):
            v1 = iz * (segments_x + 1) + ix
            v2 = v1 + 1
            v3 = v1 + segments_x + 1
            v4 = v3 + 1
            bm.faces.new([verts[v1], verts[v2], verts[v4], verts[v3]])
    
    # Create mesh and object
    mesh = bpy.data.meshes.new(name + "_mesh")
    bm.to_mesh(mesh)
    bm.free()
    
    obj = bpy.data.objects.new(name, mesh)
    collection.objects.link(obj)
    
    # Add material
    mesh.materials.append(material)
    
    # Create UV layer for the face texture
    uv_layer = mesh.uv_layers.new(name="UVMap")
    for poly in mesh.polygons:
        for loop_index in poly.loop_indices:
            co = mesh.vertices[mesh.loops[loop_index].vertex_index].co
            # Map to full texture space (0-1) - face texture has eyes and mouth in center
            # Use proportional mapping to avoid stretching on curved surface
            u = (co.x + face_width / 2) / face_width
            v = (co.z + face_height / 2) / face_height
            # Scale to 70% of texture to leave margins and avoid edge stretching
            u = 0.5 + (u - 0.5) * 0.7
            v = 0.5 + (v - 0.5) * 0.7
            uv_layer.data[loop_index].uv = (u, v)
    
    # Connect texture to material
    tree = material.node_tree
    image_node = tree.nodes.new("ShaderNodeTexImage")
    image_node.image = texture
    image_node.interpolation = "Cubic"
    image_node.location = (-420, 220)
    bsdf = next(n for n in tree.nodes if n.type == "BSDF_PRINCIPLED")
    tree.links.new(image_node.outputs["Color"], bsdf.inputs["Base Color"])
    
    # Smooth shading
    for poly in mesh.polygons:
        poly.use_smooth = True
    
    return obj


def create_face_plane(name, collection, material, texture):
    """Create a flat face plane with proper UV mapping for the texture."""
    bm = bmesh.new()
    
    # Create a square plane for the face (matches 1:1 texture aspect ratio)
    face_size = HEAD_W * 0.4  # 40% of head width (square)
    face_width = face_size
    face_height = face_size
    
    # Create plane vertices (facing -Y direction)
    verts = [
        bm.verts.new((-face_width / 2, 0, -face_height / 2)),
        bm.verts.new((face_width / 2, 0, -face_height / 2)),
        bm.verts.new((face_width / 2, 0, face_height / 2)),
        bm.verts.new((-face_width / 2, 0, face_height / 2)),
    ]
    bm.faces.new(verts)
    
    # Create mesh and object
    mesh = bpy.data.meshes.new(name + "_mesh")
    bm.to_mesh(mesh)
    bm.free()
    
    obj = bpy.data.objects.new(name, mesh)
    collection.objects.link(obj)
    
    # Add material
    mesh.materials.append(material)
    
    # Create UV layer for the face texture - proper 0-1 mapping
    uv_layer = mesh.uv_layers.new(name="UVMap")
    for poly in mesh.polygons:
        for loop_index in poly.loop_indices:
            co = mesh.vertices[mesh.loops[loop_index].vertex_index].co
            # Map to full texture space (0-1)
            u = (co.x + face_width / 2) / face_width
            v = (co.z + face_height / 2) / face_height
            uv_layer.data[loop_index].uv = (u, v)
    
    # Connect texture to material
    tree = material.node_tree
    image_node = tree.nodes.new("ShaderNodeTexImage")
    image_node.image = texture
    image_node.interpolation = "Cubic"
    image_node.location = (-420, 220)
    bsdf = next(n for n in tree.nodes if n.type == "BSDF_PRINCIPLED")
    tree.links.new(image_node.outputs["Color"], bsdf.inputs["Base Color"])
    
    return obj


# --------------------------------------------------------------------------- #
# face texture
# --------------------------------------------------------------------------- #


def write_face_texture(path, size=1024):
    """Classic smile: two vertical oval eyes and a wide curved mouth.

    Drawn in metres over the head's flat front face, so the marks land exactly
    where the reference puts them.
    """
    os.makedirs(os.path.dirname(path), exist_ok=True)
    yy, xx = np.mgrid[0:size, 0:size].astype(np.float32)
    u = (xx + 0.5) / size
    v = (yy + 0.5) / size
    x = (u - 0.5) * HEAD_FLAT_W
    z = v * HEAD_FLAT_H
    pixel = HEAD_FLAT_W / size          # metres per texel, for the AA ramp

    def coverage(signed_distance):
        return np.clip(0.5 - signed_distance / pixel, 0.0, 1.0)

    def ellipse(cx, cz, rx, rz):
        field = ((x - cx) / rx) ** 2 + ((z - cz) / rz) ** 2 - 1.0
        return coverage(field * min(rx, rz) * 0.5)

    def arc(cx, cz, radius, half_width, half_angle):
        dx, dz = x - cx, z - cz
        distance = np.hypot(dx, dz)
        band = coverage(np.abs(distance - radius) - half_width)
        angle = np.arctan2(dz, dx)
        sector = np.abs(angle + math.pi / 2) <= half_angle
        # round caps at both ends keep the stroke from looking cut
        ends = np.zeros_like(band)
        for sign in (-1.0, 1.0):
            ex = cx + sign * radius * math.sin(half_angle)
            ez = cz - radius * math.cos(half_angle)
            ends = np.maximum(
                ends, coverage(np.hypot(x - ex, z - ez) - half_width)
            )
        return np.maximum(band * sector, ends)

    ink = np.maximum(
        ellipse(-EYE_X, EYE_Z, EYE_RX, EYE_RZ),
        ellipse(EYE_X, EYE_Z, EYE_RX, EYE_RZ),
    )
    ink = np.maximum(
        ink,
        arc(0.0, MOUTH_ZC, MOUTH_R, MOUTH_STROKE, MOUTH_ARC / 2),
    )

    base = np.array([245, 205, 48], dtype=np.float32) / 255.0
    dark = np.array([22, 22, 24], dtype=np.float32) / 255.0
    colour = base[None, None, :] * (1.0 - ink[..., None]) + dark[None, None, :] * ink[..., None]

    rgba = np.ones((size, size, 4), dtype=np.float32)
    rgba[..., :3] = colour

    image = bpy.data.images.get("PokoX_Face")
    if image is not None:
        bpy.data.images.remove(image)
    image = bpy.data.images.new("PokoX_Face", width=size, height=size, alpha=True)
    image.colorspace_settings.name = "sRGB"
    image.pixels.foreach_set(rgba.ravel())
    image.filepath_raw = path
    image.file_format = "PNG"
    image.save()
    try:
        image.pack()
    except RuntimeError:
        pass
    image.reload()
    return image


# --------------------------------------------------------------------------- #
# rig
# --------------------------------------------------------------------------- #


def build_rig(collection, bones_spec, pose):
    data = bpy.data.armatures.new("PokoX_Armature")
    rig = bpy.data.objects.new("PokoX_Rig", data)
    collection.objects.link(rig)

    for obj in bpy.context.selected_objects:
        obj.select_set(False)
    rig.select_set(True)
    bpy.context.view_layer.objects.active = rig

    bpy.ops.object.mode_set(mode="EDIT")
    edit_bones = data.edit_bones
    created = {}
    for name, head, tail, parent, deform in bones_spec:
        bone = edit_bones.new(name)
        bone.head = p(*head)
        bone.tail = p(*tail)
        bone.use_deform = deform
        if parent is not None:
            bone.parent = created[parent]
            bone.use_connect = False
        created[name] = bone
    bpy.ops.armature.select_all(action="SELECT")
    bpy.ops.armature.calculate_roll(type="GLOBAL_POS_Y")
    bpy.ops.object.mode_set(mode="OBJECT")

    data.display_type = "OCTAHEDRAL"
    rig.show_in_front = True
    rig["pokox_spec"] = json.dumps(
        {
            "pose": pose,
            "height_m": round(TARGET_HEIGHT, 4),
            "bones": [b[0] for b in bones_spec],
        }
    )
    return rig


def bind_mesh(obj, rig, bone_name):
    group = obj.vertex_groups.new(name=bone_name)
    group.add(list(range(len(obj.data.vertices))), 1.0, "REPLACE")
    modifier = obj.modifiers.new("Armature", "ARMATURE")
    modifier.object = rig
    modifier.use_vertex_groups = True
    obj.parent = rig
    obj.matrix_parent_inverse = rig.matrix_world.inverted()
    obj["pokox_bone"] = bone_name
    return obj


def attach_empty(name, position, bone_name, rig, collection, size=0.045):
    empty = bpy.data.objects.new(name, None)
    empty.empty_display_type = "PLAIN_AXES"
    empty.empty_display_size = m(size)
    collection.objects.link(empty)
    empty.parent = rig
    empty.parent_type = "BONE"
    empty.parent_bone = bone_name
    # Set position in bone-local space (relative to bone head)
    bone = rig.data.bones[bone_name]
    bone_head = bone.head_local
    local_pos = Vector(position) - bone_head
    empty.matrix_world = Matrix.Translation(bone_head + local_pos)
    empty["pokox_attachment"] = bone_name
    return empty


def attach_object_to_bone(obj, position, bone_name, rig):
    """Attach any object to a bone at a specific position."""
    obj.parent = rig
    obj.parent_type = "BONE"
    obj.parent_bone = bone_name
    
    # Set position in bone-local space (relative to bone head)
    bone = rig.data.bones[bone_name]
    bone_head = bone.head_local
    local_pos = Vector(position) - bone_head
    obj.matrix_world = Matrix.Translation(bone_head + local_pos)
    obj["pokox_attachment"] = bone_name
    return obj


# --------------------------------------------------------------------------- #
# studio
# --------------------------------------------------------------------------- #


def _point_at(obj, target):
    direction = Vector(target) - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def build_studio(scene, collection, strength=1.0):
    world = bpy.data.worlds.new(scene.name + "_World")
    scene.world = world
    world.use_nodes = True
    background = next(n for n in world.node_tree.nodes if n.type == "BACKGROUND")
    background.inputs["Color"].default_value = (0.095, 0.096, 0.101, 1.0)
    background.inputs["Strength"].default_value = 1.2

    # Large, soft, narrow-spread sources: they light the character but are
    # flagged off the floor, so the backdrop stays the flat charcoal of the
    # reference instead of a bright pool in front of the camera.
    specs = [
        # name, location, energy, size, spread (degrees)
        ("Key", (2.6, -3.8, 3.3), 215.0, 3.8, 50.0),
        ("Fill", (-3.6, -3.2, 1.7), 100.0, 5.0, 55.0),
        ("Rim", (-0.9, 3.4, 2.9), 85.0, 2.6, 45.0),
    ]
    for name, location, energy, size, spread in specs:
        light_data = bpy.data.lights.new(name, type="AREA")
        light_data.energy = energy * strength
        light_data.shape = "SQUARE"
        light_data.size = size
        if hasattr(light_data, "spread"):
            light_data.spread = math.radians(spread)
        light = bpy.data.objects.new(name, light_data)
        light.location = location
        collection.objects.link(light)
        _point_at(light, (0.0, 0.0, 1.0))

    # Dark charcoal cove, self-lit and non-reflective: the backdrop value is
    # independent of the key lights, exactly like the reference's flat plate.
    backdrop_material = _material("PokoX_Backdrop", (0.0, 0.0, 0.0, 1.0), 1.0, specular=0.0)
    backdrop_bsdf = next(
        n for n in backdrop_material.node_tree.nodes if n.type == "BSDF_PRINCIPLED"
    )
    backdrop_bsdf.inputs["Emission Color"].default_value = (
        BACKDROP_EMISSION, BACKDROP_EMISSION, BACKDROP_EMISSION * 1.04, 1.0
    )
    backdrop_bsdf.inputs["Emission Strength"].default_value = 1.0

    # Cyclorama: floor sweeping into a wall with no seam, so the render has no
    # horizon line. Normals face inward (the camera sits inside).
    radius, cove, height = 14.0, 3.0, 20.0
    profile = [(0.0, 0.0)]
    for fraction in (0.2, 0.45, 0.7, 0.88):
        profile.append((radius * fraction, 0.0))
    inner = radius - cove
    for step in range(7):
        angle = (math.pi / 2) * step / 6
        profile.append((inner + cove * math.sin(angle), cove - cove * math.cos(angle)))
    profile.append((radius, height))

    bm = _revolve(profile, 96, scale_fn=lambda x, y, z: Vector((x, y, z)))
    for face in bm.faces:
        face.normal_flip()
    backdrop = _finish_mesh("Backdrop", bm, collection, backdrop_material, shade="smooth")
    return backdrop


def add_contact_shadow(collection, emission=None, width=3.8, depth=3.8, grid=64):
    """Soft painted contact shadow on the floor.

    The backdrop is emission-only, so it cannot receive a real cast shadow.
    This plane is emission-only too and fades to exactly the backdrop value at
    its border, which makes the seam invisible while giving the character the
    grounded shadow the reference has.
    """
    if emission is None:
        emission = BACKDROP_EMISSION

    bm = bmesh.new()
    verts = {}
    for j in range(grid + 1):
        for i in range(grid + 1):
            x = -width / 2 + width * i / grid
            y = -depth / 2 + depth * j / grid
            verts[(i, j)] = bm.verts.new((x, y, 0.002))
    for j in range(grid):
        for i in range(grid):
            bm.faces.new(
                (verts[(i, j)], verts[(i, j + 1)], verts[(i + 1, j + 1)], verts[(i + 1, j)])
            )
    mesh = bpy.data.meshes.new("PokoX_ContactShadow_mesh")
    bm.to_mesh(mesh)
    bm.free()
    obj = bpy.data.objects.new("PokoX_ContactShadow", mesh)
    collection.objects.link(obj)

    attribute = mesh.color_attributes.new("Shadow", "FLOAT_COLOR", "POINT")
    for index, vert in enumerate(mesh.vertices):
        x, y = vert.co.x, vert.co.y - 0.10
        falloff = 1.0 - 0.30 * math.exp(-((x / 0.62) ** 2 + (y / 0.55) ** 2))
        attribute.data[index].color = (
            emission * falloff * 0.97,
            emission * falloff * 0.99,
            emission * falloff * 1.08,
            1.0,
        )

    material = _material("PokoX_ContactShadow", (0.0, 0.0, 0.0, 1.0), 1.0, specular=0.0)
    bsdf = next(n for n in material.node_tree.nodes if n.type == "BSDF_PRINCIPLED")
    bsdf.inputs["Emission Strength"].default_value = 1.0
    vertex_colour = material.node_tree.nodes.new("ShaderNodeVertexColor")
    vertex_colour.layer_name = "Shadow"
    material.node_tree.links.new(vertex_colour.outputs["Color"], bsdf.inputs["Emission Color"])
    mesh.materials.append(material)
    return obj


def fit_camera(camera, objects, margin=1.10):
    scene = bpy.context.scene
    # The camera matrix is only refreshed by a depsgraph update; without this
    # the fit is computed from the previous view's orientation.
    bpy.context.view_layer.update()
    points = []
    for obj in objects:
        points.extend(obj.matrix_world @ Vector(corner) for corner in obj.bound_box)
    rotation = camera.matrix_world.to_3x3()
    right = rotation @ Vector((1.0, 0.0, 0.0))
    up = rotation @ Vector((0.0, 1.0, 0.0))
    forward = rotation @ Vector((0.0, 0.0, -1.0))
    center = sum(points, Vector()) / len(points)

    extents = []
    for axis, vector in (("r", right), ("u", up), ("f", forward)):
        extents.append(max(abs((point - center).dot(vector)) for point in points))
    ext_r, ext_u, ext_f = extents

    width, height = scene.render.resolution_x, scene.render.resolution_y
    sensor = camera.data.sensor_width
    lens = camera.data.lens
    half_x = (sensor * 0.5) / lens
    half_y = (sensor * (height / max(width, 1)) * 0.5) / lens
    distance = max(ext_r / half_x, ext_u / half_y) * margin + ext_f

    camera.location = center - forward * distance
    bpy.context.view_layer.update()
    return camera


# --------------------------------------------------------------------------- #
# export
# --------------------------------------------------------------------------- #


def _operator_kwargs(operator, kwargs):
    try:
        properties = operator.get_rna_type().properties.keys()
    except AttributeError:
        return kwargs
    return {key: value for key, value in kwargs.items() if key in properties}


def _deselect_everything():
    """Clear selection in every scene.

    Selection lives per view layer, so deselecting only the active scene lets a
    selected object from another scene slip into a use_selection export.
    """
    for scene in bpy.data.scenes:
        for obj in scene.objects:
            try:
                obj.select_set(False)
            except (RuntimeError, ReferenceError):
                pass


def export_glb(path, objects):
    _deselect_everything()
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = next(o for o in objects if o.type == "ARMATURE")
    os.makedirs(os.path.dirname(path), exist_ok=True)
    kwargs = _operator_kwargs(
        bpy.ops.export_scene.gltf,
        dict(
            filepath=path,
            export_format="GLB",
            use_selection=True,
            export_apply=False,
            export_animations=False,
            export_skins=True,
            export_def_bones=False,
            export_extras=True,
            export_yup=True,
            export_cameras=False,
            export_lights=False,
        ),
    )
    bpy.ops.export_scene.gltf(**kwargs)
    return path


def export_fbx(path, objects):
    if not hasattr(bpy.ops.export_scene, "fbx"):
        return None
    _deselect_everything()
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = next(o for o in objects if o.type == "ARMATURE")
    os.makedirs(os.path.dirname(path), exist_ok=True)
    kwargs = _operator_kwargs(
        bpy.ops.export_scene.fbx,
        dict(
            filepath=path,
            use_selection=True,
            object_types={"ARMATURE", "MESH", "EMPTY"},
            apply_scale_options="FBX_SCALE_ALL",
            add_leaf_bones=False,
            bake_anim=False,
            mesh_smooth_type="FACE",
            path_mode="COPY",
            embed_textures=True,
        ),
    )
    bpy.ops.export_scene.fbx(**kwargs)
    return path


# --------------------------------------------------------------------------- #
# build
# --------------------------------------------------------------------------- #


def build_character(
    pose="arms_down",
    scene_name=None,
    out_dir=None,
    render=True,
    export=True,
    samples=64,
):
    if scene_name is None:
        scene_name = "PokoX" if pose == "arms_down" else "PokoX_Tpose"
    if out_dir is None:
        repo = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
        out_dir = os.path.join(repo, "assets", "characters", "pokox")

    blend_dir = os.path.join(out_dir)
    export_dir = os.path.join(out_dir, "export")
    render_dir = os.path.join(out_dir, "renders")
    texture_dir = os.path.join(out_dir, "textures")
    for path in (blend_dir, export_dir, render_dir, texture_dir):
        os.makedirs(path, exist_ok=True)

    # Object and datablock names are global across scenes, so a second PokoX
    # scene would force .001 suffixes on the new build and on every name it
    # exports. Each variant is saved to its own .blend as it is built, so the
    # other live variant can safely be dropped first: builds stay canonical
    # whatever order they run in.
    for generated in ("PokoX", "PokoX_Tpose"):
        stale = bpy.data.scenes.get(generated)
        if stale is not None:
            bpy.data.scenes.remove(stale)
    # Drop data left behind by an earlier build so names stay unsuffixed.
    bpy.ops.outliner.orphans_purge(
        do_local_ids=True, do_linked_ids=True, do_recursive=True
    )
    scene = bpy.data.scenes.new(scene_name)
    if bpy.context.window is not None:
        bpy.context.window.scene = scene
    bpy.context.view_layer.update()

    unit = scene.unit_settings
    unit.system = "METRIC"
    unit.scale_length = 1.0
    unit.length_unit = "METERS"
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 1400
    scene.render.resolution_y = 1750
    scene.render.film_transparent = False
    try:
        scene.view_settings.view_transform = VIEW_TRANSFORM
    except TypeError:
        pass
    scene.view_settings.exposure = EXPOSURE
    if hasattr(scene, "eevee"):
        scene.eevee.taa_render_samples = samples
        for attribute, value in (
            ("use_raytracing", True),
            ("use_shadows", True),
            ("use_fast_gi", True),
        ):
            if hasattr(scene.eevee, attribute):
                setattr(scene.eevee, attribute, value)

    root = bpy.data.collections.new("PokoX")
    scene.collection.children.link(root)
    mesh_coll = bpy.data.collections.new("PokoX_Mesh")
    root.children.link(mesh_coll)
    rig_coll = bpy.data.collections.new("PokoX_Rig")
    root.children.link(rig_coll)
    att_coll = bpy.data.collections.new("PokoX_Attachments")
    root.children.link(att_coll)
    studio_coll = bpy.data.collections.new("Studio")
    scene.collection.children.link(studio_coll)

    # ---------------------------------------------------------------- materials
    mat_yellow = _material("PokoX_Yellow", YELLOW, roughness=0.8, metallic=0.0)
    mat_red = _material("PokoX_Red", RED, roughness=0.8, metallic=0.0)
    mat_blue = _material("PokoX_Blue", BLUE, roughness=0.8, metallic=0.0)
    mat_face = _material("PokoX_Face", YELLOW, roughness=0.8, metallic=0.0)

    # ------------------------------------------------------------------- meshes
    torso = add_rounded_box(
        "PokoX_Torso", (TORSO_W, TORSO_D, TORSO_H),
        (0.0, 0.0, (TORSO_BOTTOM + TORSO_TOP) / 2), mesh_coll, mat_red,
    )

    texture_path = os.path.join(texture_dir, "pokox_face.png")
    face_texture = write_face_texture(texture_path)
    head = add_head("PokoX_Head", mesh_coll, mat_yellow, mat_face, face_texture)

    if pose == "tpose":
        arm_size = (ARM_LENGTH, ARM_D, ARM_W)
        arm_center_l = (TORSO_W / 2 + ARM_LENGTH / 2, 0.0, ARM_TOP - ARM_W / 2)
        arm_center_r = (-(TORSO_W / 2 + ARM_LENGTH / 2), 0.0, ARM_TOP - ARM_W / 2)
    else:
        arm_size = (ARM_W, ARM_D, ARM_LENGTH)
        arm_center_l = (ARM_X, 0.0, ARM_TOP - ARM_LENGTH / 2)
        arm_center_r = (-ARM_X, 0.0, ARM_TOP - ARM_LENGTH / 2)

    arm_l = add_rounded_box("PokoX_Arm_L", arm_size, arm_center_l, mesh_coll, mat_yellow)
    arm_r = add_rounded_box("PokoX_Arm_R", arm_size, arm_center_r, mesh_coll, mat_yellow)
    leg_l = add_rounded_box(
        "PokoX_Leg_L", (LEG_W, LEG_D, LEG_H), (LEG_X, 0.0, LEG_H / 2), mesh_coll, mat_blue,
    )
    leg_r = add_rounded_box(
        "PokoX_Leg_R", (LEG_W, LEG_D, LEG_H), (-LEG_X, 0.0, LEG_H / 2), mesh_coll, mat_blue,
    )

    # ---------------------------------------------------------------------- rig
    bones_spec = [list(b) for b in BONE_SPEC]
    if pose == "tpose":
        for bone in bones_spec:
            if bone[0] == "Shoulder.L":
                bone[2] = [ARM_X + ARM_LENGTH, 0.0, ARM_TOP]
            elif bone[0] == "Shoulder.R":
                bone[2] = [-(ARM_X + ARM_LENGTH), 0.0, ARM_TOP]
    bones_spec = [tuple(b) for b in bones_spec]

    rig = build_rig(rig_coll, bones_spec, pose)

    bind_mesh(torso, rig, "Hip")
    bind_mesh(head, rig, "Neck")
    bind_mesh(arm_l, rig, "Shoulder.L")
    bind_mesh(arm_r, rig, "Shoulder.R")
    bind_mesh(leg_l, rig, "Hip.L")
    bind_mesh(leg_r, rig, "Hip.R")

    attachments = {}
    face_plane = None
    for name, (position, bone) in ATTACHMENTS.items():
        if name == "ATT_Face":
            # Create face plane and attach to ATT_Face attachment point
            face_plane = create_face_plane("PokoX_Face", mesh_coll, mat_face, face_texture)
            attach_object_to_bone(face_plane, position, bone, rig)
            attachments[name] = face_plane
        else:
            attachments[name] = attach_empty(name, position, bone, rig, att_coll)

    # ------------------------------------------------------------------ studio
    build_studio(scene, studio_coll)
    add_contact_shadow(studio_coll)

    camera_data = bpy.data.cameras.new("PokoX_Camera")
    camera_data.lens = 85.0
    camera_data.sensor_width = 36.0
    # Auto sensor fit maps the 36 mm sensor to the longer side, so a portrait
    # frame would be measured as if 36 mm were its height. Pin it horizontal so
    # fit_camera's maths (half_y derived from the aspect) is exact.
    camera_data.sensor_fit = "HORIZONTAL"
    camera = bpy.data.objects.new("PokoX_Camera", camera_data)
    studio_coll.objects.link(camera)
    scene.camera = camera
    bpy.context.view_layer.update()

    character = [torso, head, arm_l, arm_r, leg_l, leg_r]

    def place(direction, target, lens):
        camera_data.lens = lens
        unit_direction = Vector(direction).normalized()
        camera.location = target + unit_direction * 8.0
        camera.rotation_euler = unit_direction.to_track_quat("Z", "Y").to_euler()
        bpy.context.view_layer.update()

    place((0.24, -1.0, 0.06), Vector((0.0, 0.0, 0.9)), 110.0)
    fit_camera(camera, character, margin=1.05)

    result = {
        "scene": scene.name,
        "pose": pose,
        "texture": texture_path,
        "objects": {},
    }

    # ----------------------------------------------------------------- renders
    if render:
        import blender_kit as bk

        views = [
            # Near-front, long lens: the reference has almost no perspective.
            ("pokox_front.png", (0.16, -1.0, 0.03), 110.0, (1000, 1250)),
            ("pokox_hero.png", (0.62, -1.0, 0.22), 85.0, (1400, 1750)),
            ("pokox_right.png", (1.0, -0.02, 0.05), 110.0, (1000, 1250)),
        ]
        for name, direction, lens, resolution in views:
            scene.render.resolution_x, scene.render.resolution_y = resolution
            place(direction, Vector((0.0, 0.0, 0.9)), lens)
            fit_camera(camera, character, margin=1.05)
            bk.render_still(
                os.path.join(render_dir, name),
                resolution=resolution,
                samples=samples,
                view_transform=VIEW_TRANSFORM,
            )

        scene.render.resolution_x, scene.render.resolution_y = (900, 900)
        place((0.22, -1.0, 0.06), Vector((0.0, 0.0, HEAD_BOTTOM + HEAD_H / 2)), 110.0)
        fit_camera(camera, [head], margin=1.12)
        bk.render_still(
            os.path.join(render_dir, "pokox_face.png"),
            resolution=(900, 900),
            samples=samples,
            view_transform=VIEW_TRANSFORM,
        )

    # ------------------------------------------------------------------ export
    export_objects = character + [rig] + list(attachments.values())
    if export:
        result["glb"] = export_glb(
            os.path.join(export_dir, f"pokox_character_{pose}.glb"), export_objects
        )
        result["fbx"] = export_fbx(
            os.path.join(export_dir, f"pokox_character_{pose}.fbx"), export_objects
        )

    # ------------------------------------------------------------------- verify
    result["report"] = verify(rig, character, attachments)

    # Delivered file stays readable in the viewport: studio lights, camera and
    # backdrop are hidden (monitor toggle, renders are unaffected) and the
    # skeleton no longer draws on top of the character.
    studio_coll.hide_viewport = True
    rig.show_in_front = False

    if export:
        blend_path = os.path.join(blend_dir, f"pokox_character_{pose}.blend")
        bpy.ops.wm.save_as_mainfile(filepath=blend_path, copy=True)
        result["blend"] = blend_path

    return result


# --------------------------------------------------------------------------- #
# verification
# --------------------------------------------------------------------------- #


def verify(rig, meshes, attachments):
    import blender_kit as bk

    scene = bpy.context.scene
    report = {
        "engine": scene.render.engine,
        "unit_scale": scene.unit_settings.scale_length,
        "bones": {},
        "meshes": {},
        "attachments": {},
        "bounds": {},
    }

    for bone in rig.data.bones:
        report["bones"][bone.name] = {
            "parent": bone.parent.name if bone.parent else None,
            "deform": bone.use_deform,
            "head": [round(v, 4) for v in bone.head_local],
            "tail": [round(v, 4) for v in bone.tail_local],
        }

    lo = Vector((1e9, 1e9, 1e9))
    hi = Vector((-1e9, -1e9, -1e9))
    for obj in meshes:
        box_lo, box_hi = bk.world_bbox(obj)
        lo = Vector((min(lo.x, box_lo.x), min(lo.y, box_lo.y), min(lo.z, box_lo.z)))
        hi = Vector((max(hi.x, box_hi.x), max(hi.y, box_hi.y), max(hi.z, box_hi.z)))
        groups = {g.name: 0.0 for g in obj.vertex_groups}
        for vert in obj.data.vertices:
            for element in vert.groups:
                groups[obj.vertex_groups[element.group].name] += element.weight
        report["meshes"][obj.name] = {
            "dims": [round(v, 4) for v in obj.dimensions],
            "min_z": round(box_lo.z, 5),
            "tris": bk.tri_count(obj),
            "groups": {k: round(v, 3) for k, v in groups.items()},
            "material": [s.material.name if s.material else None for s in obj.material_slots],
            "uv_layers": [uv.name for uv in obj.data.uv_layers],
            "parent": obj.parent.name if obj.parent else None,
            "modifiers": [mod.type for mod in obj.modifiers],
            "sharp_edges": sum(1 for e in obj.data.edges if e.use_edge_sharp),
        }

    report["bounds"] = {
        "min": [round(v, 4) for v in lo],
        "max": [round(v, 4) for v in hi],
        "size": [round(v, 4) for v in (hi - lo)],
        "height": round(hi.z - lo.z, 5),
    }

    for name, empty in attachments.items():
        location = empty.matrix_world.translation
        report["attachments"][name] = {
            "world": [round(v, 4) for v in location],
            "bone": empty.parent_bone,
            "parent_type": empty.parent_type,
        }

    material = bpy.data.materials.get("PokoX_Yellow")
    if material is not None and material.use_nodes:
        bsdf = next(n for n in material.node_tree.nodes if n.type == "BSDF_PRINCIPLED")
        report["material"] = {
            "roughness": round(bsdf.inputs["Roughness"].default_value, 3),
            "metallic": round(bsdf.inputs["Metallic"].default_value, 3),
            "specular": round(bsdf.inputs["Specular IOR Level"].default_value, 3)
            if "Specular IOR Level" in bsdf.inputs
            else None,
            "base_color_linear": [round(v, 4) for v in bsdf.inputs["Base Color"].default_value],
        }

    head = bpy.data.objects.get("PokoX_Head")
    if head is not None and len(head.data.materials) > 1:
        front = [poly for poly in head.data.polygons if poly.material_index == 1]
        report["face"] = {
            "material": head.data.materials[1].name if head.data.materials[1] else None,
            "faces": len(front),
            "area": round(sum(poly.area for poly in front), 5),
            "flat_face_m": [round(HEAD_FLAT_W, 4), round(HEAD_FLAT_H, 4)],
        }

    return report


def pose_smoke_test(rig, mesh_name="PokoX_Arm_L", degrees=35.0):
    """Rotate a bone and confirm the skinned mesh actually follows it."""
    import blender_kit as bk

    obj = bpy.data.objects[mesh_name]
    before = [round(v, 4) for v in bk.world_bbox(obj)[0]]
    bpy.context.view_layer.objects.active = rig
    bpy.ops.object.mode_set(mode="POSE")
    bone = rig.pose.bones["Shoulder.L"]
    bone.rotation_mode = "XYZ"
    bone.rotation_euler = (math.radians(degrees), 0.0, 0.0)
    bpy.context.view_layer.update()
    after = [round(v, 4) for v in bk.world_bbox(obj)[0]]
    bone.rotation_euler = (0.0, 0.0, 0.0)
    bpy.context.view_layer.update()
    bpy.ops.object.mode_set(mode="OBJECT")
    return {"before_min": before, "after_min": after, "moved": before != after}


if __name__ == "__main__":
    print(json.dumps(build_character(), indent=2))
