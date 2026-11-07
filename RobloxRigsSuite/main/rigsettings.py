import bpy
from bpy.props import (BoolProperty, PointerProperty,)
from bpy.types import (Panel, Operator, PropertyGroup)
from .. import __package__ as addonKey


class RigQuality (bpy.types.PropertyGroup):

    def viewportqual_update(self, context):
        """Update viewport quality."""
        active_obj = context.object  # Active object
        if active_obj and active_obj.type == 'ARMATURE':
            meta1 = active_obj.pose.bones.get("rigmeta1")
            if meta1:
                mapping = {
                    'V0': 0,
                    'V1': 1,
                    'V2': 2,
                    'V3': 3,
                }
                meta1["Viewport Quality"] = mapping.get(self.rrs_viewportrigqual, 1)  # Default to 1

    rrs_viewportrigqual : bpy.props.EnumProperty(
        name="",
        description= "Select viewport rig quality.",
        items= [('V0', "Compatibility", ""),
                ('V1', "Balanced", ""),
                ('V2', "High", ""),
                ('V3', "Ultra", ""),
                
        ],
        default='V1',
        update=viewportqual_update,
    )

    def renderqual_update(self, context):
        """Update render quality."""
        active_obj = context.object  # Active object
        if active_obj and active_obj.type == 'ARMATURE':
            meta1 = active_obj.pose.bones.get("rigmeta1")
            if meta1:
                mapping = {
                    'R0': 0,
                    'R1': 1,
                    'R2': 2,
                    'R3': 3,
                    'R4': 4,
                }
                meta1["Render Quality"] = mapping.get(self.rrs_finalrigqual, 3)  # Default to 3

    rrs_finalrigqual : bpy.props.EnumProperty(
        name="",
        description= "Select render rig quality.",
        items= [('R0', "Compatibility", ""),
                ('R1', "Balanced", ""),
                ('R2', "High", ""),
                ('R3', "Ultra", ""),
                ('R4', "Maximum Detail", "Uses 4 subdivisions on the rig. Ideal for cinematic use (may impact render peformance).", "SEQUENCE", 4),

        ],
        default='R3',
        update= renderqual_update
    )

class RotationTool_PG (bpy.types.PropertyGroup):
    rotationEnum : bpy.props.EnumProperty(
    name="",
    description= "Select rotation mode:",
    items= [('RM1', "XYZ Euler (recommended)", ""),
            ('RM2', "XZY Euler", ""),
            ('RM3', "YXZ Euler", ""),
            ('RM4', "YZX Euler", ""),
            ('RM5', "ZXY Euler", ""),
            ('RM6', "ZYX Euler", ""),
            ('', '', '', '', 0), #(separator)
            ('RM7', "Quaternion (WXYZ)", ""),
            ('RM8', "Axis Angle", ""),
    ],
    default='RM1',
)

class RemoveRig (bpy.types.Operator):
    """Deletes the actively selected rig."""
    bl_label = "Delete Rig" #Remove Active Rig
    bl_idname = "rrs.deleterig"
    bl_description = "Deletes the actively selected rig."
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        scene = context.scene
        activerrsrig = context.object
        prefs = bpy.context.preferences.addons[addonKey].preferences
        
        meta1 = activerrsrig.pose.bones.get("rigmeta1")
        rrs_mstr_cllct_value = meta1.get("MasterCollect")
        #-Check
        if rrs_mstr_cllct_value:
            pass
        if not rrs_mstr_cllct_value:
            print ('Roblox Rigs Suite: Unsuccessfully deleted rig - rig collection could not be found!')
            self.report({'ERROR'}, "Could not delete rig - rig collection could not be found!")
            return {'CANCELLED'}
        #-Check
        rrs_mstr_cllct_name = rrs_mstr_cllct_value.name
        rrs_master_collection = bpy.data.collections.get(rrs_mstr_cllct_name)

        meta2 = activerrsrig.pose.bones.get("rigmeta2")
        rrs_mat_value = meta2.get("MatObject")
        rrs_mat_name = rrs_mat_value.name
        rrs_mat = bpy.data.materials.get(rrs_mat_name)
        
        bpy.data.materials.remove(rrs_mat)

        for obj in activerrsrig.children:
            if prefs.unlinkRigs:
                mesh = obj.data
                bpy.data.objects.remove(obj, do_unlink= True)
                
                if mesh and mesh.users == 0:
                    bpy.data.meshes.remove(mesh, do_unlink=True)
            else: bpy.data.objects.remove(obj)
        
        if prefs.unlinkRigs:
            armature_data = activerrsrig.data
            bpy.data.armatures.remove(armature_data)

        for obj in rrs_master_collection.objects:
            if prefs.unlinkRigs: bpy.data.objects.remove(obj, do_unlink= True)
            else: bpy.data.objects.remove(obj)

        for collection in rrs_master_collection.children:
            for obj in collection.objects:
                if prefs.unlinkRigs:
                    mesh = obj.data
                    bpy.data.objects.remove(obj, do_unlink= True)
                
                    if mesh and mesh.users == 0:
                        bpy.data.meshes.remove(mesh, do_unlink=True)
                else: bpy.data.objects.remove(obj)
            bpy.data.collections.remove(collection, do_unlink=True)
        bpy.data.collections.remove(rrs_master_collection, do_unlink=True)
        
        print ('Roblox Rigs Suite: Successfully deleted rig.')
        self.report({'INFO'}, "Successfully deleted rig.")
        return {'FINISHED'}
    
    def draw(self, context):
        layout = self.layout
        scene = context.scene

        layout.label(text="Are you sure you want to delete the rig?")

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self)

class RestoreDefaults (bpy.types.Operator):
    """Resets the rig back to its default settings."""
    bl_label = "Restore Defaults"
    bl_idname = "rrs.restoredefaults"
    bl_description = "Resets the rig back to its default settings."
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        scene = context.scene
        activerrsrig = context.object
        rrs_rq = activerrsrig.rrsrqregister

        full_reset = True
        
        meta1 = activerrsrig.pose.bones["rigmeta1"]
        meta2 = activerrsrig.pose.bones["rigmeta2"]
        meta3 = activerrsrig.pose.bones["rigmeta3"]

        activerrsrig_props = {
            "show_in_front": False
        }

        meta1_props = {
            "Bypass Limits": False,
            "Show Flip Bone": True,
            "Show Rig Colors": True,
            "Smooth Fix": True,
            "Show Fingers": True,
            "HideLeftFingertips": False,
            "HideRightFingertips": False,
            "Viewport Quality": 1.0,
            "Render Quality": 3.0,
            "LeftFingerAutoCurl": 0.0,
            "LeftThumbAutoCurl": 0.0,
            "RightFingerAutoCurl": 0.0,
            "RightThumbAutoCurl": 0.0,
        }

        meta2_props = {
            "Apply Bump": False,
            "Glossy": False,
        }

        meta3_props = {
            "LAFW": 0.0,
            "LAIK": 0.0,
            "LLFF": 0.0,
            "LLIK": 0.0,
            "RAFW": 0.0,
            "RAIK": 0.0,
            "RLFF": 0.0,
            "RLIK": 0.0,
        }

        #Active Rig Props
        for prop, default_value in activerrsrig_props.items():
            if hasattr(activerrsrig, prop):
                setattr(activerrsrig, prop, default_value)

        #Meta 1
        for prop, default_value in meta1_props.items():
            if prop in meta1:
                meta1[prop] = default_value
        #Meta 2
        if meta2["driverEligible"]:
            for prop, default_value in meta2_props.items():
                if prop in meta2:
                    meta2[prop] = default_value
        else:
            full_reset = False

        #Meta 3
        for prop, default_value in meta3_props.items():
            if prop in meta3:
                meta3[prop] = default_value
        rrs_rq.rrs_finalrigqual = 'R3'
        rrs_rq.rrs_viewportrigqual = 'V1'

        if full_reset == False:
            print ('Roblox Rigs Suite: Could not reset material settings as its compatibility was removed.')
            self.report({'WARNING'}, "Could not reset material settings as its compatibility was removed.")
        return {'FINISHED'}

class RotationModeSwitcher (bpy.types.Operator):
    """Changes the rotation mode of all bones to your selection."""
    bl_label = "Set Rotation Mode"
    bl_idname = "rrs.rotation_mode"
    bl_description = "Changes the rotation mode of all bones to your selection."
    bl_options = {'REGISTER', 'UNDO'}

    def execute (self, context):
        scene = context.scene
        active_obj = context.object
        rt_enum_pg = scene.rtPGrrs
        rtEnum = rt_enum_pg.rotationEnum

        rotationModesMapping = {
            'RM1': 'XYZ',
            'RM2': 'XZY',
            'RM3': 'YXZ',
            'RM4': 'YZX',
            'RM5': 'ZXY',
            'RM6': 'ZYX',
            'RM7': 'QUATERNION',
            'RM8': 'AXIS_ANGLE',
        }

        chosenMode = rotationModesMapping.get(rtEnum)

        for bone in active_obj.pose.bones:
            bone.rotation_mode = chosenMode

        self.report({'INFO'}, "Successfully changed all controls' rotation mode!")
        return {'FINISHED'}

class RigSettingsPanel (bpy.types.Panel):
    bl_label = "Rig Settings"
    bl_idname = "OBJECT_PT_rrs1"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Roblox Rigs Suite'
    # Remove this line of code if you find it is conflicting with the rest of what you're trying to do.

    def draw (self, context):
        layout = self.layout
        scene = context.scene
        active_obj = context.object

        if active_obj and active_obj.type == 'ARMATURE' and "RRS" in active_obj.name:
            meta1 = active_obj.pose.bones.get("rigmeta1")
            rigID = meta1.get("rigID")
            rrs_rq = active_obj.rrsrqregister

            rrsCol = meta1["MasterCollect"]

            #-Rig Manager
            prefs = bpy.context.preferences.addons[addonKey].preferences
            if prefs.rigManager:
                rigbox = layout.box()
                rigbox.label(text="Rig Manager:")
                if rrsCol:
                    rigbox.prop(rrsCol, "name")
                else:
                    rigbox.label(text="Missing rig collection!", icon='ERROR')
                
                rigRow = rigbox.row(align=True)
                rigRow.operator("rrs.deleterig", icon='TRASH')
                rigRow.operator("rrs.restoredefaults", text="", icon='LOOP_BACK')

                layout.separator()
            else:
                pass

            #-The rest of it
            if "g2" not in rigID:
                viewportbox = layout.box()
                renderbox = layout.box()

                viewportbox.label(text="Viewport Rig Quality:")
                viewportbox.prop(rrs_rq, "rrs_viewportrigqual")
                viewportbox.prop(meta1, '["Smooth Fix"]', text="Smooth Fix")
                layout.separator()
                renderbox.label(text="Render Rig Quality:")
                renderbox.prop(rrs_rq, "rrs_finalrigqual")
                renderbox.label(text="Changes won't apply in viewport.", icon= 'INFO')
            else:
                pass
            #
            layout.label(text="General Settings:")
            layout.prop(meta1, '["Show Flip Bone"]', text="Show Flip Bone")
            layout.prop(meta1, '["Show Rig Colors"]', text="Show Rig Colors")

            showfingersprop = meta1.get("Show Fingers")
            if showfingersprop is not None:
                layout.prop (meta1, '["Show Fingers"]', text= "Show Fingers")
            else:
                pass
            layout.prop(active_obj, "show_in_front", text="Show In Front")
        else:
            layout.label(text="No rig selected.", icon='CANCEL')
            pass

class HandSettings (bpy.types.Panel):
    bl_label = "Hand Settings"
    bl_idname = "OBJECT_PT_rrs1-hands"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Roblox Rigs Suite'
    bl_parent_id = "OBJECT_PT_rrs1"
    bl_options = {'DEFAULT_CLOSED'}

    def draw (self, context):
        layout = self.layout
        scene = context.scene
        active_obj = context.object

        if active_obj and active_obj.type == 'ARMATURE' and "RRS" in active_obj.name:
            meta1 = active_obj.pose.bones.get("rigmeta1")
            rigID = meta1.get("rigID")

            if rigID in {"rrs5g1", "rrs6g1"}:
                lfingerbox = layout.box()
                rfingerbox = layout.box()

                lfingerbox.label(text="Left Hand Settings:")
                lfingerbox.prop(meta1, '["LeftFingerAutoCurl"]', text="Finger Auto-Curl")
                lfingerbox.prop(meta1, '["LeftThumbAutoCurl"]', text="Thumb Auto-Curl")
                lfingerbox.prop(meta1, '["HideLeftFingertips"]', text="Hide Fingertips")
                layout.separator()

                rfingerbox.label(text="Right Hand Settings:")
                rfingerbox.prop(meta1, '["RightFingerAutoCurl"]', text="Finger Auto-Curl")
                rfingerbox.prop(meta1, '["RightThumbAutoCurl"]', text="Thumb Auto-Curl")
                rfingerbox.prop(meta1, '["HideRightFingertips"]', text="Hide Fingertips")
            else:
                layout.label(text="Use R6 boy/girl rig for hand settings.", icon='CANCEL')
                pass
        else:
            pass

class AdvancedRigSettings (bpy.types.Panel):
    bl_label = "Advanced"
    bl_idname = "OBJECT_PT_rrs1adv"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Roblox Rigs Suite'
    bl_parent_id = 'OBJECT_PT_rrs1'
    bl_options = {'DEFAULT_CLOSED'} # Remove this line of code if you find it is conflicting with the rest of what you're trying to do.

    def draw (self, context):
        layout = self.layout
        scene = context.scene
        active_obj = context.object

        if active_obj and active_obj.type == 'ARMATURE':
            meta1 = active_obj.pose.bones.get("rigmeta1")
            rigID = meta1.get("rigID")
            rrs_rq = active_obj.rrsrqregister
            rt_enum_pg = scene.rtPGrrs
            
            rtToolBox = layout.box()
            rtToolBox.label(text="Rotation Mode:")
            rtToolBox.prop(rt_enum_pg, "rotationEnum")
            rtToolBox.operator("rrs.rotation_mode", icon='DRIVER_ROTATIONAL_DIFFERENCE')

            layout.prop (meta1, '["Bypass Limits"]', text="Bypass Limits")
            if "g2" not in rigID:
                layout.prop (meta1, '["Render Smooth Fix"]')
        else:
            pass

#There are optional properties for rigs that have the eligibility for them;
#these include:
#   - Show fingers
#   - Finger auto curl (float slider values in their own sub-panel)

classes = [RigQuality, RotationTool_PG, RemoveRig, RestoreDefaults, RigSettingsPanel, HandSettings, RotationModeSwitcher, AdvancedRigSettings]


def register():
    for cls in (classes):
        bpy.utils.register_class(cls)
    bpy.types.Object.rrsrqregister = bpy.props.PointerProperty(type= RigQuality)
    bpy.types.Scene.rtPGrrs = bpy.props.PointerProperty(type= RotationTool_PG)


def unregister():
    for cls in (classes):
        bpy.utils.unregister_class(cls)
    del bpy.types.Scene.rtPGrrs
    del bpy.types.Object.rrsrqregister

if __name__ == "__main__":
    register()