import bpy
from ... import __package__ as addonKey

class AccessorySelector (bpy.types.PropertyGroup):
    accessory_type : bpy.props.EnumProperty(
        name="",
        description= "Select accesory type.",
        items= [('HEAD', "Head Accessory", "Includes hair/hat/neck/face accessories."),
                ('ARMS', "Shoulder Accessory", "Includes left and right shoulder accessories."),
                ('T1', "Front Accessory", ""),
                ('T2', "Back Accessory", ""),
                ('HIP', "Waist Accessory", ""),
                ('HANDS', "Gear Accessory", ""),
        ],
        default='HEAD',
    )
    
    shoulder_type : bpy.props.EnumProperty(
        name="",
        description= "Select shoulder.",
        items= [('S_LEFT', "Left Shoulder", ""),
                ('S_RIGHT', "Right Shoulder", ""),
        ],
        default='S_LEFT',
    )
    
    hand_type : bpy.props.EnumProperty(
        name="",
        description= "Select hand.",
        items= [('H_LEFT', "Left Hand", ""),
                ('H_RIGHT', "Right Hand", ""),
        ],
        default='H_LEFT',
    )

    link_accMats : bpy.props.BoolProperty(
        name="Link Materials",
        description="Links the accessory materials so that accessories have the same material as the selected rig.",
        default=False
    )

    moveToCollect : bpy.props.BoolProperty(
        name="Add To Rig",
        description="Moves your accessory to the rig's collection. Improves organisation. If you delete the rig your accessory will be included.",
        default=True
    )
    
    accessory_object: bpy.props.PointerProperty(type=bpy.types.Object)

class AttachAccessoriessOPERATOR (bpy.types.Operator):
    """Attaches your accessory to the rig's corresponding body part."""
    bl_label = "Attach"
    bl_idname = "rrs.tools_attachaccessory"
    bl_description = "Attaches your accessory to the rig's corresponding body part."
    bl_options = {'REGISTER', 'UNDO'}

    @classmethod
    def poll(cls, context):
        prefs = bpy.context.preferences.addons[addonKey].preferences
        return prefs.attachAccessories

    def execute(self, context):
        scene = context.scene
        active_obj = context.object
        acc_group = scene.accessorypropgroup_register
        current_mode = active_obj.mode #record mode
        
        #Accessory Placement Info
        #bone_targets = {
        #'HEAD': "Head",
        #'ARMS_S_LEFT': "UpperArm.L",
        #'ARMS_S_RIGHT': "UpperArm.R",
        #'T1': "UpperTorso",
        #'T2': "UpperTorso",
        #'HIP': "Hip",
        #'HANDS_H_LEFT': "Hand.L",
        #'HANDS_H_RIGHT': "Hand.R",
        #}

        acc_selection = acc_group.accessory_object
        acc_type = acc_group.accessory_type
        acc_shoulder = acc_group.shoulder_type
        acc_hand = acc_group.hand_type
        acc_obj = acc_selection

        for objects in bpy.context.view_layer.objects:
            if not acc_obj or acc_obj.name not in bpy.context.view_layer.objects:
                self.report({'ERROR'}, "Object selected doesn't exist!")
                print("Roblox Rigs Suite: Object selected doesn't exist!")
                return {'CANCELLED'} # no girly that ain't happening :/

        if active_obj:
            rrs_bone_targets = {
                'HEAD': "Head",
                'ARMS_S_LEFT': "UpperArm.L",
                'ARMS_S_RIGHT': "UpperArm.R",
                'T1': "UpperTorso",
                'T2': "UpperTorso",
                'HIP': "Hip",
                'HANDS_H_LEFT': "Hand.L",
                'HANDS_H_RIGHT': "Hand.R",
            }

            # Determine bone key
            rrs_bone_key = acc_type  # Default to accessory type
            if acc_type == 'ARMS':
                rrs_bone_key = f"ARMS_{acc_shoulder}"
            elif acc_type == 'HANDS':
                rrs_bone_key = f"HANDS_{acc_hand}"

            # Debugging print for bone key
            #print(f"Generated Bone Key: {rrs_bone_key}")

            # Get corresponding bone target
            subtarget_rrsbone = rrs_bone_targets.get(rrs_bone_key, "")

            if not subtarget_rrsbone:
                self.report({'WARNING'}, f"No valid bone target found for accessory type: {acc_type}")
                return {'CANCELLED'}

            # Assign object and create constraint
            bpy.context.view_layer.objects.active = acc_obj
            acc_obj.select_set(True)

            childof_constraint = acc_obj.constraints.new(type='CHILD_OF')
            childof_constraint.target = active_obj
            childof_constraint.subtarget = subtarget_rrsbone

            bpy.ops.constraint.childof_set_inverse(constraint=childof_constraint.name, owner='OBJECT')

            # Code has been optimized by ChatGPT. This is the original code:
            #            if acc_type == 'HEAD':
            #    bpy.context.view_layer.objects.active = acc_obj
            #    acc_obj.select_set(True)
            #
            #    childof_constraint = acc_obj.constraints.new(type='CHILD_OF')
            #    childof_constraint.target = active_obj
            #    childof_constraint.subtarget = "Head"
            #elif acc_type == 'ARMS' and acc_shoulder == 'S_LEFT':
            #    bpy.context.view_layer.objects.active = acc_obj
            #    acc_obj.select_set(True)
            #
            #    childof_constraint = acc_obj.constraints.new(type='CHILD_OF')
            #    childof_constraint.target = active_obj
            #    childof_constraint.subtarget = "UpperArm.L"
            #elif acc_type == 'ARMS' and acc_shoulder == 'S_RIGHT':
            #    bpy.context.view_layer.objects.active = acc_obj
            #    acc_obj.select_set(True)
            #
            #    childof_constraint = acc_obj.constraints.new(type='CHILD_OF')
            #    childof_constraint.target = active_obj
            #    childof_constraint.subtarget = "UpperArm.R"
            #elif acc_type == 'T1':
            #    bpy.context.view_layer.objects.active = acc_obj
            #    acc_obj.select_set(True)
            #
            #    childof_constraint = acc_obj.constraints.new(type='CHILD_OF')
            #    childof_constraint.target = active_obj
            #    childof_constraint.subtarget = "UpperTorso"
            #elif acc_type == 'T2':
            #    bpy.context.view_layer.objects.active = acc_obj
            #    acc_obj.select_set(True)
            #
            #    childof_constraint = acc_obj.constraints.new(type='CHILD_OF')
            #    childof_constraint.target = active_obj
            #    childof_constraint.subtarget = "UpperTorso"
            #elif acc_type == 'HIP':
            #    bpy.context.view_layer.objects.active = acc_obj
            #    acc_obj.select_set(True)
            #
            #    childof_constraint = acc_obj.constraints.new(type='CHILD_OF')
            #    childof_constraint.target = active_obj
            #    childof_constraint.subtarget = "Hip"
            #elif acc_type == 'HANDS' and acc_hand == 'H_LEFT':
            #    bpy.context.view_layer.objects.active = acc_obj
            #    acc_obj.select_set(True)
            #
            #    childof_constraint = acc_obj.constraints.new(type='CHILD_OF')
            #    childof_constraint.target = active_obj
            #    childof_constraint.subtarget = "Hand.L"
            #elif acc_type == 'HANDS' and acc_hand == 'H_RIGHT':
            #    bpy.context.view_layer.objects.active = acc_obj
            #    acc_obj.select_set(True)
            #
            #    childof_constraint = acc_obj.constraints.new(type='CHILD_OF')
            #    childof_constraint.target = active_obj
            #    childof_constraint.subtarget = "Hand.R"
            #else:
            #    pass

            bpy.ops.constraint.childof_set_inverse(constraint=childof_constraint.name, owner='OBJECT')
            bpy.context.view_layer.objects.active = active_obj
            active_obj.select_set(True)

            #moveToCollect:
            if acc_group.moveToCollect == True:
                meta1 = active_obj.pose.bones.get("rigmeta1")
                masterCollectValue = meta1.get("MasterCollect")
                masterCollectName = masterCollectValue.name
                masterCollect = bpy.data.collections[masterCollectName]

                for collection in masterCollect.children:
                    if "Mesh" in collection.name and "MeshShapes-RRS" not in collection.name:
                        meshCol = collection
                        break
                
                for coll in acc_obj.users_collection:
                    coll.objects.unlink(acc_obj)
        
                meshCol.objects.link(acc_obj)
            else:
                pass

            if acc_group.link_accMats == True:

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
                acc_obj.select_set(True)
                bpy.context.view_layer.objects.active = acc_obj

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
            else:
                pass
        else:
            self.report({'ERROR'}, "Unsupported rig/object selected!")
            print("Roblox Rigs Suite: Unsupported rig/object selected!")
            pass
            
        if acc_group.link_accMats == False:
            if current_mode != 'OBJECT':
                bpy.ops.object.mode_set(mode='OBJECT')
            bpy.ops.object.select_all(action='DESELECT')
            active_obj.select_set(True)
            bpy.context.view_layer.objects.active = active_obj
        
        bpy.ops.object.mode_set(mode=f'{current_mode}') #mode revert
        return {'FINISHED'}

class AttachAccessoriesPANEL (bpy.types.Panel):
    bl_label = "Attach Accessories"
    bl_idname = "OBJECT_PT_rrstools_accessories"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Roblox Rigs Suite'
    bl_parent_id = 'OBJECT_PT_rrs4'
    bl_options = {'DEFAULT_CLOSED'}

    @classmethod
    def poll(cls, context):
        prefs = bpy.context.preferences.addons[addonKey].preferences
        return prefs.attachAccessories

    def draw (self, context):
        layout = self.layout
        scene = context.scene
        acc_group = scene.accessorypropgroup_register
        active_obj = context.object

        if active_obj and active_obj.type == 'ARMATURE' and "RRS" in active_obj.name:
            layout.label(text="Attach accessories.")
            layout.separator()

            selection_box = layout.box()

            selection_box.label(text="Accessory Type:")
            selection_box.prop(acc_group, "accessory_type", text="")

            if acc_group.accessory_type == 'ARMS':
                selection_box.label(text="Shoulder Type:")
                selection_box.prop(acc_group, "shoulder_type")
            elif acc_group.accessory_type == 'HANDS':
                selection_box.label(text="Hand Type:")
                selection_box.prop(acc_group, "hand_type")
            else:
                pass             

            selection_box.label(text="Accessory Object:")
            selection_box.prop(acc_group, "accessory_object", text="")

            propRow = selection_box.row()
            propRow.prop(acc_group, "moveToCollect")
            propRow.prop(acc_group, "link_accMats")

            if acc_group.accessory_object is None:
                row = layout.row()
                row.operator("rrs.tools_attachaccessory", text="Attach")
                row.enabled = False
            else:
                row = layout.row()
                row.operator("rrs.tools_attachaccessory", text="Attach")
        else:
            layout.label(text="Must be selecting a valid rig to proceed.", icon='CANCEL')
            pass

classes = [AccessorySelector, AttachAccessoriessOPERATOR, AttachAccessoriesPANEL]


def register():
    for cls in (classes):
        bpy.utils.register_class(cls)
    bpy.types.Scene.accessorypropgroup_register = bpy.props.PointerProperty(type=AccessorySelector)

def unregister():
    for cls in (classes):
        bpy.utils.unregister_class(cls)
    del bpy.types.Scene.accessorypropgroup_register

if __name__ == "__main__":
    register()