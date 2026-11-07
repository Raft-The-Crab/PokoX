import bpy
from ... import __package__ as addonKey

class AnimationLinkerPROPGROUP (bpy.types.PropertyGroup):
    send_or_receive : bpy.props.EnumProperty(
        name="",
        description="Select which way you want to link animations.",
        items= [('TOP', "Send", "Send animation from this rig to the selected rig."),
                ('BOTTOM', "Receive", "Receive animation from the selected rig to this rig."),
        ],
        default='TOP',
    )
    
    animation_rig: bpy.props.PointerProperty(type=bpy.types.Object)

class AnimationLinkerOPERATOR (bpy.types.Operator):
    """Links animations from one rig to another."""
    bl_label = "Link Animations"
    bl_idname = "rrs.tools_linkanims"
    bl_description = "Links animations from one rig to another."
    bl_options = {'REGISTER', 'UNDO'}

    @classmethod
    def poll(cls, context):
        prefs = bpy.context.preferences.addons[addonKey].preferences
        return prefs.animationLink

    def execute(self, context):
        scene = context.scene
        active_obj = context.object
        animLinker = scene.animlinker_propgroupRegister
        selectedRig = animLinker.animation_rig

        if animLinker.send_or_receive == 'TOP': #-Send
            bpy.ops.object.mode_set(mode='OBJECT')
            bpy.ops.object.select_all(action='DESELECT')
            selectedRig.select_set(True)
            bpy.context.view_layer.objects.active = selectedRig
            active_obj.select_set(True)
            bpy.context.view_layer.objects.active = active_obj

            bpy.ops.object.make_links_data(type='ANIMATION')
            bpy.ops.object.select_all(action='DESELECT')
            
            active_obj.select_set(True)
            bpy.context.view_layer.objects.active = active_obj
            if "RRS" in selectedRig.name and selectedRig.type == 'ARMATURE':
                pass
            else:
                self.report({'WARNING'}, f"Selected rig '{selectedRig.name}' was incompatible. Errors may occur.")
                print(f"Roblox Rigs Suite: Selected rig '{selectedRig.name}' was incompatible. Errors may occur. ")
                pass
        elif animLinker.send_or_receive == 'BOTTOM': #-Receive
            bpy.ops.object.mode_set(mode='OBJECT')
            bpy.ops.object.select_all(action='DESELECT')
            active_obj.select_set(True)
            bpy.context.view_layer.objects.active = active_obj
            selectedRig.select_set(True)
            bpy.context.view_layer.objects.active = selectedRig

            bpy.ops.object.make_links_data(type='ANIMATION')
            bpy.ops.object.select_all(action='DESELECT')
            
            active_obj.select_set(True)
            bpy.context.view_layer.objects.active = active_obj
            if "RRS" in selectedRig.name and selectedRig.type == 'ARMATURE':
                #rrs_rig_action = selectedRig.animation_data.action
                pass
                #if active_obj.animation_data is None:
                #    active_obj.animation_data_create()
                #
                #active_obj.animation_data.action = rrs_rig_action
            else:
                self.report({'WARNING'}, f"Selected rig '{selectedRig.name}' was incompatible. Errors may occur.")
                print(f"Roblox Rigs Suite: Selected rig '{selectedRig.name}' was incompatible. Errors may occur. ")
                pass

        return {'FINISHED'}

class AnimationLinkerPANEL (bpy.types.Panel):
    bl_label = "Animation Linker"
    bl_idname = "OBJECT_PT_rrstools_linkanimsPanel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Roblox Rigs Suite'
    bl_parent_id = "OBJECT_PT_rrs4"
    bl_options = {'DEFAULT_CLOSED'}

    @classmethod
    def poll(cls, context):
        prefs = bpy.context.preferences.addons[addonKey].preferences
        return prefs.animationLink

    def draw (self, context):
        layout = self.layout
        scene = context.scene
        animLinker = scene.animlinker_propgroupRegister
        active_obj = context.object

        if active_obj and active_obj.type == 'ARMATURE' and "RRS" in active_obj.name:
            layout.label(text="Send or receive animation between rigs.")
            layout.label(text="Only works with Roblox Rigs Suite rigs.")
            layout.separator()

            layout.label(text="Link Method:")
            layout.prop(animLinker, "send_or_receive")
            layout.label(text="Select rig:")
            layout.prop(animLinker, "animation_rig", text="")

            if animLinker.animation_rig is None:
                row = layout.row()
                row.operator("rrs.tools_linkanims")
                row.enabled = False
            else:
                row = layout.row()
                row.operator("rrs.tools_linkanims")
        else:
            layout.label(text="Must be selecting a valid rig to proceed.", icon='CANCEL')
            pass

classes = [AnimationLinkerPROPGROUP, AnimationLinkerOPERATOR, AnimationLinkerPANEL]


def register():
    for cls in (classes):
        bpy.utils.register_class(cls)
    bpy.types.Scene.animlinker_propgroupRegister = bpy.props.PointerProperty(type=AnimationLinkerPROPGROUP)

def unregister():
    for cls in (classes):
        bpy.utils.unregister_class(cls)
    del bpy.types.Scene.animlinker_propgroupRegister

if __name__ == "__main__":
    register()