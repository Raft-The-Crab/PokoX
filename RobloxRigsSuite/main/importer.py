import bpy
from bpy.props import (BoolProperty, PointerProperty,)
from bpy.types import (Panel, Operator, PropertyGroup)
from ..load_icons import get_ico
import os

#-------------------- FUNCTIONS AND OPERATIONS/OPERATORS ----------------------------------------------------------

class RigOptionsSelector (bpy.types.PropertyGroup):

    rrsps_enum : bpy.props.EnumProperty(
        name="",
        description= "Select a package.",
        items= [('BL', "Blocky", ""),
                ('MESH', "Robloxian 2.0", ""),
                ('MAN', "Man", ""),
                ('WOMAN', "Woman", ""),
                ('BOY', "Boy", ""),
                ('GIRL', "Girl", ""),
                ('SUPER', "Superhero", ""),
                
        ],
        default='BL',
    )

    rrsg_enum : bpy.props.EnumProperty(
        name="Generation",
        description= "Choose your generation (R6 will be bendy).",
        items= [('G1', "R6", ""),
                ('G2', "R15", ""),
                
        ],
        default='G1',
    )

    rrs4_legs : bpy.props.EnumProperty(
        name="",
        description="Choose legs type.",
        items= [('WOMAN_LEGS', "Default Legs", ""),
                ('BL_LEGS', "Blocky Legs", "")],
        default='WOMAN_LEGS'
    )

    rrsbase_enum : bpy.props.EnumProperty(
        name="Rig Base",
        description= "Choose your rig base.",
        items= [('SRFRB-LIGHTNINGRIGS', "Lightning Rigs (Default)", ""),]
    )

    rrsfeat_enum : bpy.props.EnumProperty(
        name="Feature Set",
        description= "Choose your rig feature set.",
        items= [('A', "Default", ""),
                
        ]
    )

    rrsgender_input_BL : bpy.props.EnumProperty(
        name="",
        description="Choose your rig's torso type.",
        items= [('MALE', "Male", ""),
                ('FEMALE', "Female", ""),
                
        ],
        default='MALE'
    )
    
    rrsrigtype_input_BL : bpy.props.EnumProperty(
        name="",
        description="Choose your rig type.",
        items= [('SMOOTH', "Smooth", "Has a smooth look."),
                ('SHARP', "Sharp", "Has a sharp but original Roblox look.")],
        default='SMOOTH'
    )

    rrshead_input_man : bpy.props.EnumProperty(
        name="",
        description="Choose a head type.",
        items= [('DEF', "Default", ""),
                ('MANHEAD', "Man", "")],
        default='MANHEAD'
    )

    rrshead_input_woman : bpy.props.EnumProperty(
        name="",
        description="Choose a head type.",
        items= [('DEF', "Default", ""),
                ('WOMANHEAD', "Woman", "")],
        default='WOMANHEAD'
    )

    customFaces : bpy.props.BoolProperty(
        name="Use Custom Faces",
        description="Enables custom face textures on the rig, so you can use the same clothing textures with a different face texture/decal.",
        default=False,
    )

    basicMat : bpy.props.BoolProperty(
        name="Basic Materials",
        description="Uses a more basic material on rigs for more performance and manual control. Will not feature material customizations.",
        default=False,
    )

    g2_variant : bpy.props.EnumProperty(
        name="Variant",
        description="The type of R15 rig to use. Legacy will support older R15 textures.",
        items=[('DEF', "Default (2025)", "Will support newer R15 textures."),
               ('G2LEG', "Legacy", "Will support older R15 textures.")],
        default='DEF'
    )

#---- Importing Information ---------

rrs_relative = os.path.dirname(os.path.abspath(__file__))
rrs_root = os.path.dirname(rrs_relative)
rrs_path = os.path.join(rrs_root, "rigs")
rrs_directory = os.path.normpath(rrs_path)

#--- Importer ---------

class RRSRigImporter (bpy.types.Operator):
    """Imports a Roblox rig based on the settings you have picked."""
    bl_label = "Import Rig"
    bl_idname = "rrs.importrig"
    bl_description = "Imports a Roblox rig based on the settings you have picked."
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        scene = context.scene
        #-Rig Info R6
        rrs1g1 = os.path.join(rrs_directory, "BlockyR6.blend")
        rrs1g1sharp = os.path.join(rrs_directory, "BlockySharpR6.blend")
        rrs1g1fem = os.path.join(rrs_directory, "BlockyFemaleR6.blend")
        rrs1g1sharpfem = os.path.join(rrs_directory, "BlockySharpFemaleR6.blend")
        rrs2g1 = os.path.join(rrs_directory, "Robloxian_2.0_Mesh_R6.blend")
        rrs3g1 = os.path.join(rrs_directory, "ManR6.blend")
        rrs3g1def = os.path.join(rrs_directory, "ManDefaultR6.blend")
        rrs4g1 = os.path.join(rrs_directory, "WomanR6.blend")
        rrs4g1def = os.path.join(rrs_directory, "WomanDefaultR6.blend")
        rrs4g1_alt = os.path.join(rrs_directory, "WomanAltLegsR6.blend")
        rrs4g1_alt_def = os.path.join(rrs_directory, "WomanDefaultAltLegsR6.blend")
        rrs5g1 = os.path.join(rrs_directory, "BoyR6.blend")
        rrs6g1 = os.path.join(rrs_directory, "GirlR6.blend")
        rrs7g1 = os.path.join(rrs_directory, "SuperheroR6.blend")
        #-Rig Info R15
        rrs1g2 = os.path.join(rrs_directory, "BlockyR15.blend")
        rrs1g2fem = os.path.join(rrs_directory, "BlockyFemaleR15.blend")
        rrs2g2 = os.path.join(rrs_directory, "Robloxian_2.0_Mesh_R15.blend")
        rrs3g2 = os.path.join(rrs_directory, "ManR15.blend")
        rrs3g2def = os.path.join(rrs_directory, "ManDefaultR15.blend")
        rrs4g2 = os.path.join(rrs_directory, "WomanR15.blend")
        rrs4g2def = os.path.join(rrs_directory, "WomanDefaultR15.blend")
        rrs4g2_alt = os.path.join(rrs_directory, "WomanAltLegsR15.blend")
        rrs4g2_alt_def = os.path.join(rrs_directory, "WomanDefaultAltLegsR15.blend")
        rrs5g2 = os.path.join(rrs_directory, "BoyR15.blend")
        rrs6g2 = os.path.join(rrs_directory, "GirlR15.blend")
        rrs7g2 = os.path.join(rrs_directory, "SuperheroR15.blend")
        #-Collection Info R6
        collection_rrs1g1 = "Blocky R6 (RRS Rig)"
        collection_rrs1g1sharp = "Blocky Sharp R6 (RRS Rig)"
        collection_rrs1g1fem = "Blocky Female R6 (RRS Rig)"
        collection_rrs1g1sharpfem = "Blocky Sharp Female R6 (RRS Rig)"
        collection_rrs2g1 = "Robloxian 2.0 R6 (RRS Rig)"
        collection_rrs3g1 = "Man R6 (RRS Rig)"
        collection_rrs4g1 = "Woman R6 (RRS Rig)"
        collection_rrs4g1_alt = "Woman Alt R6 (RRS Rig)"
        collection_rrs5g1 = "Boy R6 (RRS Rig)"
        collection_rrs6g1 = "Girl R6 (RRS Rig)"
        collection_rrs7g1 = "Superhero R6 (RRS Rig)"
        #-Collection Info R15
        collection_rrs1g2 = {'DEF': "Blocky R15 (RRS Rig)", 'G2LEG': "Blocky Legacy R15 (RRS Rig)"}
        collection_rrs1g2fem = {'DEF': "Blocky Female R15 (RRS Rig)",
                                'G2LEG': "Blocky Female Legacy R15 (RRS Rig)"}
        collection_rrs2g2 = {'DEF': "Robloxian 2.0 R15 (RRS Rig)", 'G2LEG': "Robloxian 2.0 Legacy R15 (RRS Rig)"}
        collection_rrs3g2 = {'DEF': "Man R15 (RRS Rig)", 'G2LEG': "Man Legacy R15 (RRS Rig)"}
        collection_rrs4g2 = {'DEF': "Woman R15 (RRS Rig)", 'G2LEG': "Woman Legacy R15 (RRS Rig)"}
        collection_rrs4g2_alt = {'DEF': "Woman Alt R15 (RRS Rig)", 'G2LEG': "Woman Alt Legacy R15 (RRS Rig)"}
        collection_rrs5g2 = {'DEF': "Boy R15 (RRS Rig)", 'G2LEG': "Boy Legacy R15 (RRS Rig)"}
        collection_rrs6g2 = {'DEF': "Girl R15 (RRS Rig)", 'G2LEG': "Girl Legacy R15 (RRS Rig)"}
        collection_rrs7g2 = {'DEF': "Superhero R15 (RRS Rig)", 'G2LEG': "Superhero Legacy R15 (RRS Rig)"}
        #-Options Info
        rrs_os = scene.rrsosregister
        active_obj = context.object
        variant = "G2LEG" if rrs_os.g2_variant == "G2LEG" else "DEF"

        ## This part sorts out the selection of the rig        
        if active_obj:
            bpy.ops.object.mode_set(mode='OBJECT')
        else:
            pass
        bpy.ops.object.select_all(action='DESELECT')

        ## This part sorts out the importing of the rig
        #-Blocky Male Smooth R6
        if rrs_os.rrsps_enum == 'BL' and rrs_os.rrsg_enum == 'G1' and rrs_os.rrsgender_input_BL == 'MALE' and rrs_os.rrsrigtype_input_BL == 'SMOOTH':
            with bpy.data.libraries.load(rrs1g1, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs1g1]
            context.collection.children.link(data_to.collections[0])
        
        #-Blocky Male Sharp R6
        if rrs_os.rrsps_enum == 'BL' and rrs_os.rrsg_enum == 'G1' and rrs_os.rrsgender_input_BL == 'MALE' and rrs_os.rrsrigtype_input_BL == 'SHARP':
            with bpy.data.libraries.load(rrs1g1sharp, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs1g1sharp]
            context.collection.children.link(data_to.collections[0])
        
        #-Blocky Female Smooth R6
        if rrs_os.rrsps_enum == 'BL' and rrs_os.rrsg_enum == 'G1' and rrs_os.rrsgender_input_BL == 'FEMALE' and rrs_os.rrsrigtype_input_BL == 'SMOOTH':
            with bpy.data.libraries.load(rrs1g1fem, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs1g1fem]
            context.collection.children.link(data_to.collections[0])

        #-Blocky Female Sharp R6
        if rrs_os.rrsps_enum == 'BL' and rrs_os.rrsg_enum == 'G1' and rrs_os.rrsgender_input_BL == 'FEMALE' and rrs_os.rrsrigtype_input_BL == 'SHARP':
            with bpy.data.libraries.load(rrs1g1sharpfem, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs1g1sharpfem]
            context.collection.children.link(data_to.collections[0])
        
        #-Robloxian 2.0/Mesh R6
        if rrs_os.rrsps_enum == 'MESH' and rrs_os.rrsg_enum == 'G1':
            with bpy.data.libraries.load(rrs2g1, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs2g1]
            context.collection.children.link(data_to.collections[0])

        #-Man R6
        if rrs_os.rrsps_enum == 'MAN' and rrs_os.rrsg_enum == 'G1' and rrs_os.rrshead_input_man == 'MANHEAD':
            with bpy.data.libraries.load(rrs3g1, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs3g1]
            context.collection.children.link(data_to.collections[0])

        #-Default Man R6
        if rrs_os.rrsps_enum == 'MAN' and rrs_os.rrsg_enum == 'G1' and rrs_os.rrshead_input_man == 'DEF':
            with bpy.data.libraries.load(rrs3g1def, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs3g1]
            context.collection.children.link(data_to.collections[0])
        
        #-Woman R6
        if rrs_os.rrsps_enum == 'WOMAN' and rrs_os.rrsg_enum == 'G1' and rrs_os.rrs4_legs == 'WOMAN_LEGS' and rrs_os.rrshead_input_woman == 'WOMANHEAD':
            with bpy.data.libraries.load(rrs4g1, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs4g1]
            context.collection.children.link(data_to.collections[0])

        #-Woman Default R6
        if rrs_os.rrsps_enum == 'WOMAN' and rrs_os.rrsg_enum == 'G1' and rrs_os.rrs4_legs == 'WOMAN_LEGS' and rrs_os.rrshead_input_woman == 'DEF':
            with bpy.data.libraries.load(rrs4g1def, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs4g1]
            context.collection.children.link(data_to.collections[0])

        #-Woman Alt R6
        if rrs_os.rrsps_enum == 'WOMAN' and rrs_os.rrsg_enum == 'G1' and rrs_os.rrs4_legs == 'BL_LEGS' and rrs_os.rrshead_input_woman == 'WOMANHEAD':
            with bpy.data.libraries.load(rrs4g1_alt, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs4g1_alt]
            context.collection.children.link(data_to.collections[0])

        #-Woman Alt Default R6
        if rrs_os.rrsps_enum == 'WOMAN' and rrs_os.rrsg_enum == 'G1' and rrs_os.rrs4_legs == 'BL_LEGS' and rrs_os.rrshead_input_woman == 'DEF':
            with bpy.data.libraries.load(rrs4g1_alt_def, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs4g1_alt]
            context.collection.children.link(data_to.collections[0])

        #-Boy R6
        if rrs_os.rrsps_enum == 'BOY' and rrs_os.rrsg_enum == 'G1':
            with bpy.data.libraries.load(rrs5g1, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs5g1]
            context.collection.children.link(data_to.collections[0])

        #-Girl R6
        if rrs_os.rrsps_enum == 'GIRL' and rrs_os.rrsg_enum == 'G1':
            with bpy.data.libraries.load(rrs6g1, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs6g1]
            context.collection.children.link(data_to.collections[0])

        #-Superhero R6
        if rrs_os.rrsps_enum == 'SUPER' and rrs_os.rrsg_enum == 'G1':
            with bpy.data.libraries.load(rrs7g1, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs7g1]
            context.collection.children.link(data_to.collections[0])
        
        #===============================================================================================================

        #-Blocky R15 Normal
        if rrs_os.rrsps_enum == 'BL' and rrs_os.rrsg_enum == 'G2' and rrs_os.rrsgender_input_BL == 'MALE':
            with bpy.data.libraries.load(rrs1g2, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs1g2[variant]]
            context.collection.children.link(data_to.collections[0])
        
        #-Blocky R15 Female Normal
        if rrs_os.rrsps_enum == 'BL' and rrs_os.rrsg_enum == 'G2' and rrs_os.rrsgender_input_BL == 'FEMALE':
            with bpy.data.libraries.load(rrs1g2fem, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs1g2fem[variant]]
            context.collection.children.link(data_to.collections[0])          

        #-Robloxian 2.0/Mesh R15
        if rrs_os.rrsps_enum == 'MESH' and rrs_os.rrsg_enum == 'G2':
            with bpy.data.libraries.load(rrs2g2, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs2g2[variant]]
            context.collection.children.link(data_to.collections[0])
        
        #-Man R15
        if rrs_os.rrsps_enum == 'MAN' and rrs_os.rrsg_enum == 'G2' and rrs_os.rrshead_input_man == 'MANHEAD':
            with bpy.data.libraries.load(rrs3g2, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs3g2[variant]]
            context.collection.children.link(data_to.collections[0])
        
        #-Man Default R15
        if rrs_os.rrsps_enum == 'MAN' and rrs_os.rrsg_enum == 'G2' and rrs_os.rrshead_input_man == 'DEF':
            with bpy.data.libraries.load(rrs3g2def, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs3g2[variant]]
            context.collection.children.link(data_to.collections[0])

        #-Woman R15
        if rrs_os.rrsps_enum == 'WOMAN' and rrs_os.rrsg_enum == 'G2' and rrs_os.rrs4_legs == 'WOMAN_LEGS' and rrs_os.rrshead_input_woman == 'WOMANHEAD':
            with bpy.data.libraries.load(rrs4g2, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs4g2[variant]]
            context.collection.children.link(data_to.collections[0])
        
        #-Woman Default R15
        if rrs_os.rrsps_enum == 'WOMAN' and rrs_os.rrsg_enum == 'G2' and rrs_os.rrs4_legs == 'WOMAN_LEGS' and rrs_os.rrshead_input_woman == 'DEF':
            with bpy.data.libraries.load(rrs4g2def, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs4g2[variant]]
            context.collection.children.link(data_to.collections[0])

        #-Woman Alt R15
        if rrs_os.rrsps_enum == 'WOMAN' and rrs_os.rrsg_enum == 'G2' and rrs_os.rrs4_legs == 'BL_LEGS' and rrs_os.rrshead_input_woman == 'WOMANHEAD':
            with bpy.data.libraries.load(rrs4g2_alt, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs4g2_alt[variant]]
            context.collection.children.link(data_to.collections[0])
        
        #-Woman Alt Default R15
        if rrs_os.rrsps_enum == 'WOMAN' and rrs_os.rrsg_enum == 'G2' and rrs_os.rrs4_legs == 'BL_LEGS' and rrs_os.rrshead_input_woman == 'DEF':
            with bpy.data.libraries.load(rrs4g2_alt_def, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs4g2_alt[variant]]
            context.collection.children.link(data_to.collections[0])

        #-Boy R15
        if rrs_os.rrsps_enum == 'BOY' and rrs_os.rrsg_enum == 'G2':
            with bpy.data.libraries.load(rrs5g2, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs5g2[variant]]
            context.collection.children.link(data_to.collections[0])

        #-Girl R15
        if rrs_os.rrsps_enum == 'GIRL' and rrs_os.rrsg_enum == 'G2':
            with bpy.data.libraries.load(rrs6g2, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs6g2[variant]]
            context.collection.children.link(data_to.collections[0])

        #-Superhero R15
        if rrs_os.rrsps_enum == 'SUPER' and rrs_os.rrsg_enum == 'G2':
            with bpy.data.libraries.load(rrs7g2, link=False) as (data_from, data_to):
                data_to.collections = [collection_rrs7g2[variant]]
            context.collection.children.link(data_to.collections[0])


        bpy.ops.object.select_all(action='DESELECT')
        print ('Roblox Rigs Suite: Rig imported.')
        self.report({'INFO'}, "Rig has been imported!")
    
        return {'FINISHED'}

#--------REMOVING STUFF IF YOU WANT TO---------------------------------------------
class RemoveActiveRig (bpy.types.Operator):
    """Deletes the active selected rig."""
    bl_label = "" #Remove Active Rig
    bl_idname = "rrs.deleterig"
    bl_description = "Deletes the active selected rig."

    def execute(self, context):
        activerrsrig = context.object
        meta1 = activerrsrig.pose.bones.get("rigmeta1")
        rrs_mstr_cllct_value = meta1.get("MasterCollect")
        rrs_mstr_cllct_name = rrs_mstr_cllct_value.name
        rrs_master_collection = bpy.data.collections.get(rrs_mstr_cllct_name)


        for parent in bpy.data.collections:
            if rrs_master_collection.name in parent.children.keys():
                parent.children.unlink(rrs_master_collection)
                break
        
        bpy.data.collections.remove(rrs_master_collection)
        
        
        print ('Roblox Rigs Suite: Successfully deleted rig.')
        self.report({'INFO'}, "Successfully deleted rig.")
        return {'FINISHED'}
#-------------------- DRAWING PANELS ----------------------------------------------------------

class ImporterPanel (bpy.types.Panel):
    bl_label = "Importer"
    bl_idname = "OBJECT_PT_rrsimp"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Roblox Rigs Suite'
    bl_options = {'HEADER_LAYOUT_EXPAND'}

    def draw(self, context):
        layout = self.layout
        scene = context.scene
        rrs_os = scene.rrsosregister
        activeobj = bpy.context.active_object
        
        row = layout.row()
        layout.label(text="Welcome to Roblox Rigs Suite!")

        layout.label(text="Select a rig:")
        layout.prop(rrs_os, "rrsps_enum")
        layout.prop(rrs_os, "rrsg_enum", expand=True)
        #layout.separator()

        row = layout.row(align=True)
        row.operator("rrs.importrig", icon= 'IMPORT')

        #if activeobj is not None:
        #    if activeobj and activeobj.type == 'ARMATURE' and "RRS" in activeobj.name:
        #        row.operator("rrs.deleterig", icon='TRASH')
        #    else:
        #        pass
        #else:
        #    pass

class RRSMatSettings (bpy.types.Panel):
    bl_label = "Material Settings"
    bl_idname = "OBJECT_PT_rrsimpMat"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Roblox Rigs Suite'
    bl_parent_id = 'OBJECT_PT_rrsimp'
    bl_options = {'DEFAULT_CLOSED'}

    def draw(self, context):
        layout = self.layout
        scene = context.scene
        rrs_os = scene.rrsosregister
        
        layout.prop(rrs_os, "customFaces")
        #layout.prop()

class RRSAdvancedRigSettings(bpy.types.Panel):
    bl_idname = "OBJECT_PT_rrsimpAdv"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = "Roblox Rigs Suite"
    bl_parent_id = 'OBJECT_PT_rrsimp'
    bl_label = "Advanced"
    bl_options = {'DEFAULT_CLOSED'}

    def draw (self, context):
        layout = self.layout
        scene = context.scene
        rrs_os = scene.rrsosregister
        
        layout.prop(rrs_os, "rrsbase_enum")
        #layout.prop(rrs_os, "rrsfeat_enum")

        if rrs_os.rrsg_enum == 'G2':
            layout.prop(rrs_os, "g2_variant")

        if rrs_os.rrsps_enum == 'MAN':
            layout.separator()
            manbox = layout.box()
            manbox.label(text="Man Rig Settings", icon_value=get_ico("rrs3"))
            manbox.label(text="Head Type:")
            manbox.prop(rrs_os, "rrshead_input_man")
        elif rrs_os.rrsps_enum == 'WOMAN':
            layout.separator()
            womanbox = layout.box()
            womanbox.label(text="Woman Rig Settings", icon_value=get_ico("rrs4"))
            womanbox.label(text="Head Type:")
            womanbox.prop(rrs_os, "rrshead_input_woman")
            womanbox.label(text="Legs Type:")
            womanbox.prop(rrs_os, "rrs4_legs")
        else:
            pass

        if rrs_os.rrsps_enum == 'BL': #Blocky Rig Settings
            layout.separator()
            blockybox = layout.box()
            blockybox.label(text="Blocky Rig Settings", icon_value=get_ico("rrs1"))
            if rrs_os.rrsg_enum == 'G1':
                blockybox.label(text="Rig Type:")
                blockybox.prop(rrs_os, "rrsrigtype_input_BL")
            else:
                pass
            blockybox.label(text="Torso Type:")
            blockybox.prop(rrs_os, "rrsgender_input_BL")
        else:
            pass

# 'RRSAdvancedRigSettings' will not be registered until I can add more stuff to it (as in more options on both the rig base and feature set).

classes = [RigOptionsSelector, RRSRigImporter, ImporterPanel, RRSAdvancedRigSettings]


def register():
    for cls in (classes):
        bpy.utils.register_class(cls)
    bpy.types.Scene.rrsosregister = bpy.props.PointerProperty(type= RigOptionsSelector)


def unregister():
    for cls in (classes):
        bpy.utils.unregister_class(cls)
    del bpy.types.Scene.rrsosregister

if __name__ == "__main__":
    register()