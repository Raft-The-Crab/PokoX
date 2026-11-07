import bpy
from ... import __package__ as addonKey

# REMOVE COMPATIBILITY Definition
#
#Steps:
#Remove all bone shapes (maybe)
#
#Bake animations (visual keying on, clear constraints on, overwrite current action on, only selected bones off)
#
#Remove all edge splits (maybe)
#
#Rename rig
#
#Remove metas

class RemoveCompatibilityOPERATOR (bpy.types.Operator):
    """Removes Blender-specific features on the rig, and makes the rig universal. Can be useful for cross-platform use."""
    bl_label = "Remove Compatibility"
    bl_idname = "rrs.tools_removecompatibility"
    bl_description = "Removes Blender-specific features on the rig, and makes the rig universal. Can be useful for cross-platform use."
    bl_options = {'REGISTER'}
    
    @classmethod
    def poll(cls, context):
        prefs = bpy.context.preferences.addons[addonKey].preferences
        return prefs.advancedTools and context.object and 'ARMATURE' in context.object.type and 'RRS' in context.object.name

    def preset_update (self, context):
        if self.rcPreset == 'RC0':
            return

        if self.rcPreset == 'RC1':
            self.driverCleanUp = True
            self.removeEdgeSplits = False
            self.removeBoneShapes = False
            self.removeMatComp = True
            self.removeExtras = False
        elif self.rcPreset == 'RC2':
            self.driverCleanUp = True
            self.removeEdgeSplits = False
            self.removeBoneShapes = False
            self.removeMatComp = False
            self.removeExtras = False
        elif self.rcPreset == 'RC3':
            self.driverCleanUp = True
            self.removeEdgeSplits = True
            self.removeBoneShapes = True
            self.removeMatComp = True
            self.removeExtras = True

    rcPreset: bpy.props.EnumProperty(
        name="Preset",
        description="Select the intensity of the compatibility removal",
        items=[('RC0', "Custom", "Is based on the user's choice."),
               ('RC1', "Default", "Does a normal clean-up of the rig."),
               ('RC2', "Light Removal", "Minimally cleans up the rig (driver clean-up only)."),
               ('RC3', "Full Removal", "Fully removes the rig from the addon (all options).")],
        default='RC1',
        update= preset_update
    )
    
    bakeAnimations: bpy.props.BoolProperty(
        name="Bake Animations",
        description="Bakes the rig's animations and clears its constraints, IK animations will become FK and will make animations universal.",
        default=False,
    )

    driverCleanUp: bpy.props.BoolProperty(
        name="Driver Clean-Up",
        description="Removes any invalid drivers after removing compatibility.",
        default=True,
    )

    removeEdgeSplits: bpy.props.BoolProperty(
        name="Remove Edge Splits",
        description="Removes edge split modifiers from the rig. Edge splits may make it harder to manage normals, so enabling this may help.",
        default=False,
    )

    removeBoneShapes: bpy.props.BoolProperty(
        name="Remove Bone Shapes",
        description="This removes all bone shapes/controller styles from the rig.",
        default=False,
    )

    removeMatComp: bpy.props.BoolProperty(
        name="Remove Material Compatibility",
        description="This removes drivers in the rig's material. This allows for more manual control..",
        default=True,
    )

    removeExtras: bpy.props.BoolProperty(
        name= "Remove Extra Nodes",
        description="Gets rid of extra nodes, so you have a fully basic material.",
        default=False,
    )

    #keepPost: bpy.props.BoolProperty(
    #    name
    #)

    def execute(self, context):
        debug = False
        prefs = bpy.context.preferences.addons[addonKey].preferences
        if prefs.debugMode: debug = True


        if debug: print("Roblox Rigs Suite - Debug - REMOVE COMPATIBILITY OPERATOR")
        #-First Check
        if not (self.bakeAnimations or self.driverCleanUp or self.removeEdgeSplits or self.removeBoneShapes):
            self.report({'ERROR'}, "Invalid selection - please enable at least one property to remove compatibility!")
            return {'CANCELLED'}
        
        if debug: print("First check reported that rig is valid to proceed.")
        
        #-Definitions
        scene = context.scene
        active_obj = context.object

        rrs_armature = active_obj.data
        rrs_rigmetas = ["rigmeta1", "rigmeta2", "rigmeta3"]
        rrs_rigmetasCollect = rrs_armature.collections.get("Metas")
        
        meta1 = active_obj.pose.bones.get("rigmeta1")
        meta2 = active_obj.pose.bones.get("rigmeta2")
        meta3 = active_obj.pose.bones.get("rigmeta3")

        masterCollect = meta1["MasterCollect"]

        #-Step 1: Bake The Animation
        if self.bakeAnimations:
            frSt = bpy.data.scenes[scene.name].frame_start
            FrEnd = bpy.data.scenes[scene.name].frame_end
            FrStp = bpy.data.scenes[scene.name].frame_step

            bpy.ops.nla.bake(frame_start=frSt, frame_end=FrEnd,
                              step=FrStp, only_selected=False, visual_keying=True,)
            
            if debug: print("Rig animation has been baked.")

        #-Step 2: Material Cleanup
        if self.removeMatComp:
            driverProp = meta2["driverEligible"]

            rrs_materialBLOCK = meta2.get("MatObject")
            rrs_materialNAME = rrs_materialBLOCK.name
            rrs_material = bpy.data.materials.get(rrs_materialNAME)
            rrs_nodeTree = rrs_material.node_tree

            principledNode = None
            bumpNode = None
            groupNode = None
            mixNode = None
            imageNode = None

            for node in rrs_nodeTree.nodes:
                if node.type == 'BSDF_PRINCIPLED':
                    principledNode = node
                if node.type == 'BUMP':
                    bumpNode = node
                if node.type == 'GROUP':
                    groupNode = node
                if node.type == 'MIX':
                    mixNode = node
                if node.type == 'TEX_IMAGE':
                    imageNode = node

            if principledNode is not None:
                for i, socket in enumerate(principledNode.inputs):
                        if socket.is_linked:
                            continue  # skip linked sockets
                        if socket.driver_add:
                            try:
                                principledNode.inputs[i].driver_remove("default_value")
                            except:
                                pass

            if bumpNode is not None:            
                for i, socket in enumerate(bumpNode.inputs):
                        if socket.is_linked:
                            continue  # skip linked sockets
                        if socket.driver_add:
                            try:
                                bumpNode.inputs[i].driver_remove("default_value")
                            except:
                                pass
                        
            if self.removeExtras == True:
                if bumpNode:
                    rrs_nodeTree.nodes.remove(bumpNode)
                if groupNode:
                    rrs_nodeTree.nodes.remove(groupNode)
                if mixNode:
                    rrs_nodeTree.nodes.remove(mixNode)
                rrs_nodeTree.links.new(imageNode.outputs["Color"], principledNode.inputs["Base Color"])
                meta2["ccFaces"] = False

            meta2["driverEligible"] = False
            if debug: print("Materials were disconnected from the addon.")
        
        #-Step 3: Remove Bone Shapes
        if self.removeBoneShapes:
            bpy.ops.object.mode_set(mode='POSE')
            for bone in active_obj.pose.bones:
                bone.custom_shape = None

                bone.custom_shape_scale_xyz = (1.0, 1.0, 1.0)
                bone.custom_shape_translation = (0.0, 0.0, 0.0)
                bone.custom_shape_rotation_euler = (0.0, 0.0, 0.0)
            bpy.ops.object.mode_set(mode='OBJECT')
            
            for collection in masterCollect.children:
                if "Shapes" in collection.name and "-RRS" in collection.name:
                    boneShapeCollect = collection
            for obj in boneShapeCollect.objects:
                bpy.data.objects.remove(obj)
            if debug: print("Bone shapes were not removed.")

        #-Step 4: Driver Cleanup
        if self.driverCleanUp:
            for obj in active_obj.children:
                for modifier in obj.modifiers:
                    mod_props = [prop.identifier for prop in modifier.bl_rna.properties if not prop.is_readonly]
                    for prop in mod_props:
                        try:
                            modifier.driver_remove(prop)
                        except TypeError:
                            pass
            
            aramture_props = [prop.identifier for prop in rrs_armature.bl_rna.properties if not prop.is_readonly]
            for prop in aramture_props:
                try:
                    rrs_armature.driver_remove(prop)
                except TypeError:
                    pass  # Ignore if the property does not have a driver
            
            for bone in active_obj.pose.bones:
                bone_props = [prop.identifier for prop in bone.bl_rna.properties if not prop.is_readonly]

                for prop in bone_props:
                    try:
                        bone.driver_remove(prop)
                    except TypeError:
                        pass  # Ignore if the property does not have a driver

                for constraint in bone.constraints:
                    constraint_props = [prop.identifier for prop in constraint.bl_rna.properties if not prop.is_readonly]
                    for prop in constraint_props:
                        try:
                            constraint.driver_remove(prop)
                        except TypeError:
                            pass  # Ignore if the property does not have a driver
            print("Drivers were removed.")
        else:
            if debug: print("Drivers were purposely not removed.")
            pass
        
        #-Step 5: Remove Edge Splits
        if self.removeEdgeSplits:
            for obj in active_obj.children:
                for modifier in list(obj.modifiers):
                    if modifier.type == 'EDGE_SPLIT':
                        obj.modifiers.remove(modifier)
            if debug: print("Edge splits removed.")

        #-Step 6: Removing Metas
        bpy.ops.object.mode_set(mode='EDIT')
        
        for bone_name in rrs_rigmetas:
            if bone_name in rrs_armature.edit_bones:
                rrs_armature.edit_bones.remove(rrs_armature.edit_bones[bone_name])
        
        bpy.ops.object.mode_set(mode='POSE')
        rrs_armature.collections.remove(rrs_rigmetasCollect)
        if debug: print("Rig metas successfully removed.")

        #-Step 7: Rename Rig Items
        ogRigName = masterCollect.name
        if "RRS" in masterCollect.name:
            masterCollect.name = masterCollect.name.replace("RRS", "").strip()
            for obj in active_obj.children:
                if "-RRS" in obj.name: obj.name = obj.name.replace("-RRS", "").strip()
                if "RRS" in obj.name: obj.name = obj.name.replace("RRS", "").strip()

        active_obj.name = active_obj.name.replace("RRS", "").strip() #-FINAL DISCONNECT
        if debug: print("Final disconnect from rig to addon has been completed.")

        self.report({'INFO'}, f"Removed compatibility from '{ogRigName}'. It will not work with the addon anymore.")
        return {'FINISHED'}

    def draw(self, context):
        layout = self.layout
        scene = context.scene

        layout.label(text="This will remove all RRS-relevant features from the")
        layout.label(text="selected active rig and make it universal. This action")
        layout.label(text="permanently modifies the rig. Do you wish to proceed?")
        layout.separator()

        layout.prop(self, "rcPreset")

        if self.rcPreset == 'RC0':
            layout.prop(self, "driverCleanUp")
            layout.prop(self, "removeEdgeSplits")
            layout.prop(self, "removeBoneShapes")
            layout.separator()

        if self.rcPreset == 'RC0':
            layout.prop(self, "removeMatComp")
            if self.removeMatComp: layout.prop(self, "removeExtras")
            layout.prop(self, "bakeAnimations")
        if self.rcPreset == 'RC1' or self.rcPreset == 'RC3':
            layout.prop(self, "bakeAnimations")

        if self.bakeAnimations: layout.label(text="Baking animations could slow down the process.", icon='WARNING_LARGE')

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self)

classes = [RemoveCompatibilityOPERATOR]


def register():
    for cls in (classes):
        bpy.utils.register_class(cls)

def unregister():
    for cls in (classes):
        bpy.utils.unregister_class(cls)
    
if __name__ == "__main__":
    register()