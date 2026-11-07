import bpy
from ... import __package__ as addonKey

class MasterHeadSelector (bpy.types.PropertyGroup):
    master_head_object: bpy.props.PointerProperty(type=bpy.types.Object)
    linkObjMaterials : bpy.props.BoolProperty(
        name="Link Materials",
        description="Links the selected object's materials with the rig so both objects are affected when changes are made.",
        default=False
    )

class AttachCustomHeadsOPERATOR (bpy.types.Operator):
    """Binds your custom head to the rig."""
    bl_label = "Bind"
    bl_idname = "rrs.tools_bindhead"
    bl_description = "Binds your custom head to the rig."
    bl_options = {'REGISTER', 'UNDO'}

    @classmethod
    def poll(cls, context):
        prefs = bpy.context.preferences.addons[addonKey].preferences
        return prefs.attachCustomHeads

    def execute(self, context):
        scene = context.scene
        active_obj = context.object
        head_selection_group = scene.headpropgroup_register
        rrs_head_bone = active_obj.pose.bones.get("Head")
        head_selection = head_selection_group.master_head_object
        head_obj = head_selection

        if active_obj and rrs_head_bone and head_obj:
            bpy.context.view_layer.objects.active = head_obj
            head_obj.select_set(True)
            
            childof_constraint = head_obj.constraints.new(type='CHILD_OF')
            childof_constraint.target = active_obj
            childof_constraint.subtarget = "Head"

            bpy.ops.constraint.childof_set_inverse(constraint=childof_constraint.name, owner='OBJECT')

            if head_selection_group.linkObjMaterials == True:
                la_obj = None
                ll_obj = None

                #find left arm obj
                for child in active_obj.children: #-Left Arm
                    if "LA-RRS" in child.name:
                        la_obj = child
                        break
                    if "LL-RRS" in child.name: #-Backup use Left Leg
                        ll_obj = child
                        break

                bpy.ops.object.mode_set(mode='OBJECT')
                active_rrs = active_obj

                bpy.ops.object.select_all(action='DESELECT')
                head_obj.select_set(True)
                bpy.context.view_layer.objects.active = head_obj

                if la_obj:
                    la_obj.select_set(True)
                    bpy.context.view_layer.objects.active = la_obj
                    bpy.ops.object.make_links_data(type='MATERIAL')

                    bpy.ops.object.select_all(action='DESELECT')
                    active_rrs.select_set(True)
                    bpy.context.view_layer.objects.active = active_rrs
                elif ll_obj:
                    ll_obj.select_set(True)
                    bpy.context.view_layer.objects.active = ll_obj
                    bpy.ops.object.make_links_data(type='MATERIAL')

                    bpy.ops.object.select_all(action='DESELECT')
                    active_rrs.select_set(True)
                    bpy.context.view_layer.objects.active = active_rrs
                else:
                    self.report({'ERROR'}, "No valid body part to link materials with!")
                    print("Roblox Rigs Suite: No valid body part to link materials with!")
                    pass
            
            #original_head_scale = head_obj.scale.copy()
            #childof_constraint.inverse_matrix = head_obj.matrix_world.inverted()
            #head_obj.scale = original_head_scale
        else:
            self.report({'ERROR'}, "Unsupported rig/object selected!")
            print(text="Roblox Rigs Suite: Unsupported rig/object selected!")
            pass


        return {'FINISHED'}

class AttachCustomHeadsPANEL (bpy.types.Panel):
    bl_label = "Attach Custom Head/Face Rig"
    bl_idname = "OBJECT_PT_rrstools_customheads"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Roblox Rigs Suite'
    bl_parent_id = 'OBJECT_PT_rrs4'
    bl_options = {'DEFAULT_CLOSED'}

    @classmethod
    def poll(cls, context):
        prefs = bpy.context.preferences.addons[addonKey].preferences
        return prefs.attachCustomHeads

    def draw (self, context):
        layout = self.layout
        scene = context.scene
        active_obj = context.object

        if active_obj and active_obj.type == 'ARMATURE' and "RRS" in active_obj.name:
            headTypeSelectionBLOCK = scene.headpropgroup_register.master_head_object
            headTypeSelection = None
            if headTypeSelectionBLOCK is not None:
                headTypeSelectionNAME = headTypeSelectionBLOCK.name
                headTypeSelection = bpy.data.objects.get(headTypeSelectionNAME)
            else:
                pass

            layout.label(text="Attach custom heads or face rigs.")
            layout.separator()

            layout.label(text="Master head/face rig object:")
            layout.prop(scene.headpropgroup_register, "master_head_object", text="")

            if headTypeSelection:
                if headTypeSelection.type != 'MESH':
                    col = layout.column()
                    col.enabled = False
                    col.prop(scene.headpropgroup_register, "linkObjMaterials")
                    pass
                elif headTypeSelection and headTypeSelection.type == 'MESH':
                    col = layout.column()
                    col.prop(scene.headpropgroup_register, "linkObjMaterials")
                else:
                    pass
            else:
                pass

            if scene.headpropgroup_register.master_head_object is None:
                row = layout.row()
                row.operator("rrs.tools_bindhead", text="Bind")
                row.enabled = False
            else:
                row = layout.row()
                row.operator("rrs.tools_bindhead", text="Bind")
        else:
            layout.label(text="Must be selecting a valid rig to proceed.", icon='CANCEL')
            pass

classes = [MasterHeadSelector, AttachCustomHeadsOPERATOR, AttachCustomHeadsPANEL]


def register():
    for cls in (classes):
        bpy.utils.register_class(cls)
    bpy.types.Scene.headpropgroup_register = bpy.props.PointerProperty(type=MasterHeadSelector)

def unregister():
    for cls in (classes):
        bpy.utils.unregister_class(cls)
    del bpy.types.Scene.headpropgroup_register

if __name__ == "__main__":
    register()