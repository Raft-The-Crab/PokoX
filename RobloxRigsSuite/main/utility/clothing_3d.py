import bpy
from ... import __package__ as addonKey

#-Surface Deform Method:
#
#-Optimize Geometry (ALT+J then M by distance)
#-Add surface deform modifier
#-Set modifier target
#-Bind modifier

class ClothingCustomizer (bpy.types.PropertyGroup):
    bind_method : bpy.props.EnumProperty(
        name="",
        description="Select which binding method you would like to use to attach your 3D clothing to the rig.",
        items= [('SURF_DEF', "Surface Deform", "Uses the surface deform modifier to copy the deformations the body makes to rig the 3D clothing. Method by imbob02."),
                ('TRANSFER_DATA', "Data Transfer", "Uses the data transfer modifer to transfer weight information to the clothing and rig it automatically. Method by CubeBite."),
                ('AUTO_WEIGHT', "Automatic Weights", "Uses the Blender native automatic weighting to rig 3D clothing. May be unreliable.")],
        default= 'SURF_DEF',
    )

    bp : bpy.props.EnumProperty(
        name="",
        description="Select which body part the 3D clothing should be binded to.",
        items= [('LA', "Left Arm", ""),
                ('LL', "Left Leg", ""),
                ('RA', "Right Arm", ""),
                ('RL', "Right Leg", ""),
                ('T', "Torso", "")],
        default='LA'
    )

    clothing_type : bpy.props.EnumProperty(
        name="",
        description="Select the type of 3D clothing.",
        items= [('TOP', "Tops", "Includes shirts, jackets, etc."),
                ('BOTTOM', "Bottoms", "Includes pants, shorts, etc."),
                ('DR', "Dress", ""),
                ('SK', "Skirt", ""),
                ('SHOES', "Shoes", "")],
        default='TOP'
    )

    shoe_side : bpy.props.EnumProperty(
        name="",
        description="Select which foot to assign the shoe object to.",
        items= [('LF', "Left Foot", ""),
                ('RF', "Right Foot", "")],
        default='LF'
    )

    do_it_on_Ultra : bpy.props.BoolProperty(
        name="Bind On Ultra",
        description="Binds the selected clothing models when the rig is at the highest quality, so when rendered the 3D clothing is still rigged.",
        default=False
    )

    clothing_object: bpy.props.PointerProperty(type=bpy.types.Object)

class Clothing3DOPERATOR (bpy.types.Operator):
    """Binds your 3D clothing to the rig."""
    bl_label = "Bind"
    bl_idname = "rrs.tools_bindclothing"
    bl_description = "Binds your 3D clothing to the rig."
    bl_options = {'REGISTER', 'UNDO'}

    @classmethod
    def poll(cls, context):
        prefs = bpy.context.preferences.addons[addonKey].preferences
        return prefs.layeredClothing

    def execute(self, context):
        scene = context.scene
        active_obj = context.object
        meta1 = active_obj.pose.bones.get("rigmeta1")
        cBind = scene.clothinggroup_register
        cObj = cBind.clothing_object

        #-Step 1: find the body parts
        for child in active_obj.children: #-Head
            if "H-RRS" in child.name:
                head_obj = child
                break
        
        for child in active_obj.children: #-Left Arm
            if "LA-RRS" in child.name:
                la_obj = child
                break
        
        for child in active_obj.children: #-Left Leg
            if "LL-RRS" in child.name:
                ll_obj = child
                break
        
        for child in active_obj.children: #-Right Arm
            if "RA-RRS" in child.name:
                ra_obj = child
                break
        
        for child in active_obj.children: #-Right Leg
            if "RL-RRS" in child.name:
                rl_obj = child
                break

        for child in active_obj.children: #-Torso
            if "T-RRS" in child.name:
                t_obj = child
                break
        
        if cBind.bind_method == 'SURF_DEF':
            if cBind.do_it_on_Ultra == True:
                meta1["Viewport Quality"] = 3
                active_obj.rrsrqregister.rrs_viewportrigqual = 'V3'
            else:
                pass

            bpy.ops.object.mode_set(mode='OBJECT')
            bpy.ops.object.select_all(action='DESELECT')

            cObj.select_set(True)
            bpy.context.view_layer.objects.active = cObj
            active_obj.select_set(True)
            bpy.context.view_layer.objects.active = active_obj
            bpy.ops.object.parent_set(type='OBJECT', keep_transform=False)

            cObj.select_set(True)
            bpy.context.view_layer.objects.active = cObj
            bpy.ops.object.mode_set(mode='EDIT')
            bpy.ops.mesh.select_all(action='SELECT')

            bpy.ops.mesh.tris_convert_to_quads()
            bpy.ops.mesh.remove_doubles()
            
            bpy.ops.object.mode_set(mode='OBJECT')
            bpy.ops.mesh.customdata_custom_splitnormals_clear()
            
            surf_def = cObj.modifiers.new(name="LayeredClothingAttachment", type='SURFACE_DEFORM')

            if cBind.bp == 'LA':
                surf_def.target = la_obj
            elif cBind.bp == 'LL':
                surf_def.target = ll_obj
            elif cBind.bp == 'RA':
                surf_def.target = ra_obj
            elif cBind.bp == 'RL':
                surf_def.target = rl_obj
            elif cBind.bp == 'T':
                surf_def.target = t_obj
            
            bpy.ops.object.surfacedeform_bind(modifier=surf_def.name)
        elif cBind.bind_method == 'AUTO_WEIGHT':
            bpy.ops.object.mode_set(mode='OBJECT')
            active_rrs = active_obj

            bpy.ops.object.select_all(action='DESELECT')
            cObj.select_set(True)
            bpy.context.view_layer.objects.active = cObj
            
            bpy.ops.object.mode_set(mode='EDIT')
            bpy.ops.mesh.select_all(action='SELECT')
            bpy.ops.mesh.tris_convert_to_quads()
            bpy.ops.mesh.remove_doubles()
            bpy.ops.object.mode_set(mode='OBJECT')
            bpy.ops.mesh.customdata_custom_splitnormals_clear()

            active_rrs.select_set(True)
            bpy.context.view_layer.objects.active = active_rrs

            bpy.ops.object.parent_set(type='ARMATURE_AUTO')
        elif cBind.bind_method == 'TRANSFER_DATA':
            bpy.ops.object.mode_set(mode='OBJECT') #-Step 1: Fix Geometry
            active_rrs = active_obj

            bpy.ops.object.select_all(action='DESELECT')
            cObj.select_set(True)
            bpy.context.view_layer.objects.active = cObj
            
            bpy.ops.object.mode_set(mode='EDIT')
            bpy.ops.mesh.select_all(action='SELECT')
            bpy.ops.mesh.tris_convert_to_quads()
            bpy.ops.mesh.remove_doubles()
            bpy.ops.object.mode_set(mode='OBJECT')
            bpy.ops.mesh.customdata_custom_splitnormals_clear()

            if cBind.clothing_type == 'TOP':
                cObj.parent = active_rrs  # Set parent to active_rrs
                cObj.parent_type = 'ARMATURE'  # Set the parent type to Armature Deform #-Step 2: Parent to Armature
                dt_la = cObj.modifiers.new(name="LayeredClothingAttachment_LA", type='DATA_TRANSFER')
                dt_t = cObj.modifiers.new(name="LayeredClothingAttachment_T", type='DATA_TRANSFER')
                dt_ra = cObj.modifiers.new(name="LayeredClothingAttachment_RA", type='DATA_TRANSFER')

                cObj.modifiers[dt_la.name].object = la_obj
                cObj.modifiers[dt_t.name].object = t_obj
                cObj.modifiers[dt_ra.name].object = ra_obj

                cObj.modifiers[dt_la.name].use_vert_data = True
                cObj.modifiers[dt_t.name].use_vert_data = True
                cObj.modifiers[dt_ra.name].use_vert_data = True

                cObj.modifiers[dt_la.name].data_types_verts = {'VGROUP_WEIGHTS'}
                cObj.modifiers[dt_t.name].data_types_verts = {'VGROUP_WEIGHTS'}
                cObj.modifiers[dt_ra.name].data_types_verts = {'VGROUP_WEIGHTS'}
                cObj.modifiers[dt_la.name].vert_mapping = 'POLYINTERP_NEAREST'
                cObj.modifiers[dt_t.name].vert_mapping = 'POLYINTERP_NEAREST'
                cObj.modifiers[dt_ra.name].vert_mapping = 'POLYINTERP_NEAREST'

                bpy.ops.object.datalayout_transfer(modifier="LayeredClothingAttachment_LA")
                bpy.ops.object.datalayout_transfer(modifier="LayeredClothingAttachment_T")
                bpy.ops.object.datalayout_transfer(modifier="LayeredClothingAttachment_RA")

                bpy.ops.object.modifier_apply(modifier="LayeredClothingAttachment_LA")
                bpy.ops.object.modifier_apply(modifier="LayeredClothingAttachment_T")
                bpy.ops.object.modifier_apply(modifier="LayeredClothingAttachment_RA")
            elif cBind.clothing_type == 'BOTTOM':
                cObj.parent = active_rrs  # Set parent to active_rrs
                cObj.parent_type = 'ARMATURE'  # Set the parent type to Armature Deform #-Step 2: Parent to Armature
                dt_ll = cObj.modifiers.new(name="LayeredClothingAttachment_LL", type='DATA_TRANSFER')
                dt_rl = cObj.modifiers.new(name="LayeredClothingAttachment_RL", type='DATA_TRANSFER')

                dt_ll.object = ll_obj
                dt_rl.object = rl_obj

                cObj.modifiers[dt_ll.name].use_vert_data = True
                cObj.modifiers[dt_rl.name].use_vert_data = True

                bpy.context.object.modifiers[dt_ll.name].data_types_verts = {'VGROUP_WEIGHTS'}
                bpy.context.object.modifiers[dt_rl.name].data_types_verts = {'VGROUP_WEIGHTS'}
                bpy.context.object.modifiers[dt_ll.name].vert_mapping = 'POLYINTERP_NEAREST'
                bpy.context.object.modifiers[dt_rl.name].vert_mapping = 'POLYINTERP_NEAREST'

                bpy.ops.object.datalayout_transfer(modifier="LayeredClothingAttachment_LL")
                bpy.ops.object.datalayout_transfer(modifier="LayeredClothingAttachment_RL")

                bpy.ops.object.modifier_apply(modifier="LayeredClothingAttachment_LL")
                bpy.ops.object.modifier_apply(modifier="LayeredClothingAttachment_RL")
            elif cBind.clothing_type == 'SK':
                cObj.parent = active_rrs  # Set parent to active_rrs
                cObj.parent_type = 'ARMATURE'  # Set the parent type to Armature Deform #-Step 2: Parent to Armature #-Step 2: Parent to Armature
                dt_ll = cObj.modifiers.new(name="LayeredClothingAttachment_LL", type='DATA_TRANSFER')
                dt_rl = cObj.modifiers.new(name="LayeredClothingAttachment_RL", type='DATA_TRANSFER')

                dt_ll.object = ll_obj
                dt_rl.object = rl_obj

                cObj.modifiers[dt_ll.name].use_vert_data = True
                cObj.modifiers[dt_rl.name].use_vert_data = True

                bpy.context.object.modifiers[dt_ll.name].data_types_verts = {'VGROUP_WEIGHTS'}
                bpy.context.object.modifiers[dt_rl.name].data_types_verts = {'VGROUP_WEIGHTS'}
                bpy.context.object.modifiers[dt_ll.name].vert_mapping = 'POLYINTERP_NEAREST'
                bpy.context.object.modifiers[dt_rl.name].vert_mapping = 'POLYINTERP_NEAREST'

                bpy.ops.object.datalayout_transfer(modifier="LayeredClothingAttachment_LL")
                bpy.ops.object.datalayout_transfer(modifier="LayeredClothingAttachment_RL")

                bpy.ops.object.modifier_apply(modifier="LayeredClothingAttachment_LL")
                bpy.ops.object.modifier_apply(modifier="LayeredClothingAttachment_RL")
            elif cBind.clothing_type == 'DR':
                cObj.parent = active_rrs  # Set parent to active_rrs
                cObj.parent_type = 'ARMATURE'  # Set the parent type to Armature Deform #-Step 2: Parent to Armature #-Step 2: Parent to Armature
                dt_la = cObj.modifiers.new(name="LayeredClothingAttachment_LA", type='DATA_TRANSFER')
                dt_t = cObj.modifiers.new(name="LayeredClothingAttachment_T", type='DATA_TRANSFER')
                dt_ra = cObj.modifiers.new(name="LayeredClothingAttachment_RA", type='DATA_TRANSFER')
                dt_ll = cObj.modifiers.new(name="LayeredClothingAttachment_LL", type='DATA_TRANSFER')
                dt_rl = cObj.modifiers.new(name="LayeredClothingAttachment_RL", type='DATA_TRANSFER')

                dt_la.object = la_obj
                dt_t.object = t_obj
                dt_ra.object = ra_obj
                dt_ll.object = ll_obj
                dt_rl.object = rl_obj

                cObj.modifiers[dt_la.name].use_vert_data = True
                cObj.modifiers[dt_t.name].use_vert_data = True
                cObj.modifiers[dt_ra.name].use_vert_data = True
                cObj.modifiers[dt_ll.name].use_vert_data = True
                cObj.modifiers[dt_rl.name].use_vert_data = True

                bpy.context.object.modifiers[dt_la.name].data_types_verts = {'VGROUP_WEIGHTS'}
                bpy.context.object.modifiers[dt_t.name].data_types_verts = {'VGROUP_WEIGHTS'}
                bpy.context.object.modifiers[dt_ra.name].data_types_verts = {'VGROUP_WEIGHTS'}
                bpy.context.object.modifiers[dt_ll.name].data_types_verts = {'VGROUP_WEIGHTS'}
                bpy.context.object.modifiers[dt_rl.name].data_types_verts = {'VGROUP_WEIGHTS'}
                bpy.context.object.modifiers[dt_la.name].vert_mapping = 'POLYINTERP_NEAREST'
                bpy.context.object.modifiers[dt_t.name].vert_mapping = 'POLYINTERP_NEAREST'
                bpy.context.object.modifiers[dt_ra.name].vert_mapping = 'POLYINTERP_NEAREST'
                bpy.context.object.modifiers[dt_ll.name].vert_mapping = 'POLYINTERP_NEAREST'
                bpy.context.object.modifiers[dt_rl.name].vert_mapping = 'POLYINTERP_NEAREST'

                bpy.ops.object.datalayout_transfer(modifier="LayeredClothingAttachment_LA")
                bpy.ops.object.datalayout_transfer(modifier="LayeredClothingAttachment_T")
                bpy.ops.object.datalayout_transfer(modifier="LayeredClothingAttachment_RA")
                bpy.ops.object.datalayout_transfer(modifier="LayeredClothingAttachment_LL")
                bpy.ops.object.datalayout_transfer(modifier="LayeredClothingAttachment_RL")

                bpy.ops.object.modifier_apply(modifier="LayeredClothingAttachment_LA")
                bpy.ops.object.modifier_apply(modifier="LayeredClothingAttachment_T")
                bpy.ops.object.modifier_apply(modifier="LayeredClothingAttachment_RA")
                bpy.ops.object.modifier_apply(modifier="LayeredClothingAttachment_LL")
                bpy.ops.object.modifier_apply(modifier="LayeredClothingAttachment_RL")
            elif cBind.clothing_type == 'SHOES' and cBind.shoe_side == 'LF':
                active_rrs.select_set(True)
                bpy.context.view_layer.objects.active = active_rrs
                bpy.ops.object.mode_set(mode='POSE')

                active_obj.data.bones["Foot.L"].select = True
                active_obj.data.bones.active = active_obj.data.bones["Foot.L"]
                bpy.ops.object.parent_set(type='BONE')
                bpy.ops.object.mode_set(mode='OBJECT')
            elif cBind.clothing_type == 'SHOES' and cBind.shoe_side == 'RF':
                active_rrs.select_set(True)
                bpy.context.view_layer.objects.active = active_rrs
                bpy.ops.object.mode_set(mode='POSE')

                active_obj.data.bones["Foot.R"].select = True
                active_obj.data.bones.active = active_obj.data.bones["Foot.R"]
                bpy.ops.object.parent_set(type='BONE')
                bpy.ops.object.mode_set(mode='OBJECT')
        else:
            pass
        return {'FINISHED'}

class Clothing3DPANEL (bpy.types.Panel):
    bl_label = "3D/Layered Clothing"
    bl_idname = "OBJECT_PT_rrstools_clothing"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Roblox Rigs Suite'
    bl_parent_id = 'OBJECT_PT_rrs4'
    bl_options = {'DEFAULT_CLOSED'}

    @classmethod
    def poll(cls, context):
        prefs = bpy.context.preferences.addons[addonKey].preferences
        return prefs.layeredClothing

    def draw (self, context):
        layout = self.layout
        scene = context.scene
        active_obj = context.object
        cBind = scene.clothinggroup_register
        if active_obj and active_obj.type == 'ARMATURE':
            meta1 = active_obj.pose.bones.get("rigmeta1")
            rigID = meta1.get("rigID")
        else:
            pass

        if active_obj and active_obj.type == 'ARMATURE' and "RRS" in active_obj.name and "g1" in rigID:
            layout.label(text="Automatically rig 3D/layered clothing to your rig.")
            layout.separator()

            cBox = layout.box()

            cBox.label(text="Binding Method:")
            cBox.prop(cBind, "bind_method")

            if cBind.bind_method == 'SURF_DEF':
                cBox.label(text="Body Part:")
                cBox.prop(cBind, "bp")
            else:
                pass
            
            if cBind.bind_method == 'TRANSFER_DATA':
                cBox.label(text="Clothing Type:")
                cBox.prop(cBind, "clothing_type")
            else:
                pass

            if cBind.bind_method == 'TRANSFER_DATA':
                if cBind.clothing_type == 'SHOES':
                    cBox.label(text="Shoe Side:")
                    cBox.prop(cBind, "shoe_side")
                else:
                    pass

            cBox.label(text="Clothing Object:")
            cBox.prop(cBind, "clothing_object", text="")

            if cBind.bind_method == 'SURF_DEF':
                cBox.prop(cBind, "do_it_on_Ultra")
            else:
                pass

            if cBind.clothing_object is None:
                row = layout.row()
                row.operator("rrs.tools_bindclothing", text="Bind")
                row.enabled = False
            else:
                row = layout.row()
                row.operator("rrs.tools_bindclothing", text="Bind")
            
            if cBind.bind_method == 'TRANSFER_DATA':
                layout.label(text="May be unreliable.", icon='WARNING_LARGE')
            else:
                pass

        elif active_obj and active_obj.type == 'ARMATURE' and "RRS" in active_obj.name and "g2" in rigID:
            layout.label(text="3D clothing auto-rigging only works on R6 rigs.", icon='CANCEL')
        else:
            layout.label(text="Must be selecting a valid rig to proceed.", icon='CANCEL')
            pass

class Clothing3DHelpPANEL (bpy.types.Panel):
    bl_label = "Help"
    bl_idname = "OBJECT_PT_rrstools_clothingHelp"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Roblox Rigs Suite'
    bl_parent_id = 'OBJECT_PT_rrstools_clothing'
    bl_options = {'DEFAULT_CLOSED'}

    @classmethod
    def poll(cls, context):
        prefs = bpy.context.preferences.addons[addonKey].preferences
        return prefs.layeredClothing

    def draw (self, context):
        layout = self.layout
        scene = context.scene

        layout.label(text="Help", icon='QUESTION')
        layout.separator()

        layout.label(text="This tool allows you to bind 3D/layered")
        layout.label(text="clothing to the rig. While this tool is not")
        layout.label(text="perfect, it will allow you to quickly attach")
        layout.label(text="clothing without (hopefully) too many issues.")
        layout.separator()

        layout.label(text="If you are using the surface deform method,")
        layout.label(text="make sure to separate clothing objects")
        layout.label(text="before binding.")
        layout.separator()

        layout.label(text="There are plans to improve the data transfer")
        layout.label(text="method, as it has potential. Stay tuned on")
        layout.label(text="Comix Productions socials to keep updated!")

classes = [ClothingCustomizer, Clothing3DOPERATOR, Clothing3DPANEL, Clothing3DHelpPANEL]


def register():
    for cls in (classes):
        bpy.utils.register_class(cls)
    bpy.types.Scene.clothinggroup_register = bpy.props.PointerProperty(type=ClothingCustomizer)

def unregister():
    for cls in (classes):
        bpy.utils.unregister_class(cls)
    del bpy.types.Scene.clothinggroup_register

if __name__ == "__main__":
    register()