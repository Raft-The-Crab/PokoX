import bpy

class PoleFollow (bpy.types.Operator):
    """Toggles the pole follow on the rig."""
    bl_label = "Toggle Pole Follow"
    bl_idname = "rrs.pole_follow"
    bl_description = "Toggles the pole follow on the rig."
    bl_options = {'REGISTER', 'UNDO'}

    def execute (self, context):
        scene = context.scene
        rig = context.object
        meta1 = rig.pose.bones.get("rigmeta1")
        meta3 = rig.pose.bones.get("rigmeta3")

        poleFollowProp = meta3["PoleFollow"]
        rigID = meta1["rigID"]

        #RRS1 Settings Arms
        translateY_rrs1_arms = -0.463
        translateZ_rrs1_arms = -1.51
        rotationY_rrs1_arms = -1.5708
        
        #RRS1 Settings Legs
        translateY_rrs1_legs = -1.00741
        translateZ_rrs1_legs = 1.51
        rotationY_rrs1_legs = 1.5708

        lh = rig.pose.bones.get("Hand.L")
        rh = rig.pose.bones.get("Hand.R")
        lf = rig.pose.bones.get("Foot.L")
        rf = rig.pose.bones.get("Foot.R")

        if meta3["PoleFollow"] == False:
            if "rrs1" in rigID:
                for bone in rig.pose.bones:
                    if "ArmPole.L" in bone.name:
                        bone.custom_shape_transform = lh
                        bone.custom_shape_translation[1] = translateY_rrs1_arms
                        bone.custom_shape_translation[2] = translateZ_rrs1_arms
                        bone.custom_shape_rotation_euler[1] = rotationY_rrs1_arms
                    if "ArmPole.R" in bone.name:
                        bone.custom_shape_transform = rh
                        bone.custom_shape_translation[1] = translateY_rrs1_arms
                        bone.custom_shape_translation[2] = translateZ_rrs1_arms
                        bone.custom_shape_rotation_euler[1] = rotationY_rrs1_arms
                    if "LegPole.L" in bone.name:
                        bone.custom_shape_transform = lf
                        bone.custom_shape_translation[1] = translateY_rrs1_legs
                        bone.custom_shape_translation[2] = translateZ_rrs1_legs
                        bone.custom_shape_rotation_euler[1] = rotationY_rrs1_legs
                    if "LegPole.R" in bone.name:
                        bone.custom_shape_transform = rf
                        bone.custom_shape_translation[1] = translateY_rrs1_legs
                        bone.custom_shape_translation[2] = translateZ_rrs1_legs
                        bone.custom_shape_rotation_euler[1] = rotationY_rrs1_legs
                meta3["PoleFollow"] = True
        else:
            for bone in rig.pose.bones:
                if "ArmPole.L" in bone.name:
                    bone.custom_shape_transform = None
                    bone.custom_shape_translation[1] = 0
                    bone.custom_shape_translation[2] = 0
                    bone.custom_shape_euler[1] = 0
                if "ArmPole.R" in bone.name:
                    bone.custom_shape_transform = None
                    bone.custom_shape_translation[1] = 0
                    bone.custom_shape_translation[2] = 0
                    bone.custom_shape_euler[1] = 0
                if "LegPole.L" in bone.name:
                    bone.custom_shape_transform = None
                    bone.custom_shape_translation[1] = 0
                    bone.custom_shape_translation[2] = 0
                    bone.custom_shape_euler[1] = 0
                if "LegPole.R" in bone.name:
                    bone.custom_shape_transform = None
                    bone.custom_shape_translation[1] = 0
                    bone.custom_shape_translation[2] = 0
                    bone.custom_shape_euler[1] = 0
            meta3["PoleFollow"] = False
        return {'FINISHED'}

class IKSettingsPanel (bpy.types.Panel):
    bl_label = "IK Settings"
    bl_idname = "OBJECT_PT_rrs3"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Roblox Rigs Suite'
    bl_options = {'DEFAULT_CLOSED'} # Remove this line of code if you find it is conflicting with the rest of what you're trying to do.

    def draw (self, context):
        layout = self.layout
        scene = context.scene
        active_obj = context.object

        if active_obj and active_obj.type == 'ARMATURE' and "RRS" in active_obj.name:
            meta3 = active_obj.pose.bones.get("rigmeta3")

            #IKFKSwitch
            layout.label (text="Left Arm IK:")
            layout.prop (meta3, '["LAIK"]', text="IK Factor")
            layout.label (text="Left Leg IK:")
            layout.prop (meta3, '["LLIK"]', text="IK Factor")
            layout.label (text="Right Arm IK:")
            layout.prop (meta3, '["RAIK"]', text="IK Factor")
            layout.label (text="Right Leg IK:")
            layout.prop (meta3, '["RLIK"]', text="IK Factor")
            #layout.operator ("rrs.pole_follow")
        else:
            layout.label(text="No rig selected.", icon='CANCEL')
            pass

class FFSwitch (bpy.types.Panel):
    bl_label = "Fancy Wrists/Feet (IK Parent)"
    bl_idname = "OBJECT_PT_rrs3-ff"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Roblox Rigs Suite'
    bl_parent_id = 'OBJECT_PT_rrs3'
    bl_options = {"DEFAULT_CLOSED"}

    def draw (self, context):
        layout = self.layout
        scene = context.scene
        active_obj = context.object

        if active_obj and active_obj.type == 'ARMATURE' and "RRS" in active_obj.name:
            meta3 = active_obj.pose.bones.get("rigmeta3")
            meta1 = active_obj.pose.bones.get("rigmeta1")
            rigID = meta1.get("rigID")

            leftfwprop = meta3.get("LAFW")
            rightfwprop = meta3.get("RAFW")
            

            layout.label (text="Left Fancy Wrist:")
            layout.prop (meta3, '["LAFW"]', text="FW Factor")
            
            layout.label (text="Left Fancy Foot:")
            layout.prop (meta3, '["LLFF"]', text="FF Factor")
            
            layout.label (text="Right Fancy Wrist:")
            layout.prop (meta3, '["RAFW"]', text="FW Factor")

            layout.label (text="Right Fancy Foot:")
            layout.prop (meta3, '["RLFF"]', text="FF Factor")
        else:
            pass

#Some rigs do not have fancy wrists (i.e mesh rig), therefore there is convention to make sure it 'fails' silently if those properties can't be found.

classes = [IKSettingsPanel, FFSwitch]

def register():
    for cls in (classes):
        bpy.utils.register_class(cls)


def unregister():
    for cls in (classes):
        bpy.utils.unregister_class(cls)

if __name__ == "__main__":
    register()