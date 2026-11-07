import bpy

#bpy.types.Scene.toggleMappingEditUI = bpy.props.BoolProperty(
#    name="Edit Mapping",
#    description="Allows you to edit the mapping of the custom face texture.",
#    default=False
#)

class BasicMaterials (bpy.types.Operator):
    """Removes the material's compatibility with the addon. This allows for more manual control."""
    bl_label = "Remove Material Compatibility"
    bl_idname = "rrs.removematerialcompatibility"
    bl_description = "Removes the material's compatibility with the addon. This allows for more manual control."
    bl_options = {'REGISTER', 'UNDO'}

    removeExtras: bpy.props.BoolProperty(
        name= "Remove Extra Nodes",
        description="Gets rid of extra nodes, so you have a fully basic material.",
        default=False
    )

    def execute (self, context):
        scene = context.scene
        activerrsrig = context.object
        meta2 = activerrsrig.pose.bones.get("rigmeta2")
        driverProp = meta2["driverEligible"]
        #
        rrs_materialBLOCK = meta2.get("MatObject")
        rrs_materialNAME = rrs_materialBLOCK.name
        rrs_material = bpy.data.materials.get(rrs_materialNAME)
        rrs_nodeTree = rrs_material.node_tree
        #
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

        for i, socket in enumerate(principledNode.inputs):
                if socket.is_linked:
                    continue  # skip linked sockets
                if socket.driver_add:
                    try:
                        principledNode.inputs[i].driver_remove("default_value")
                    except:
                        pass
        
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

        print ('Roblox Rigs Suite: Successfully removed material compatibility.')
        self.report({'INFO'}, "Successfully removed material compatibility.")
        return {'FINISHED'}

    def draw(self, context):
            layout = self.layout
            scene = context.scene

            layout.label(text="This all addon removes compatibility from the material.")
            layout.label(text="Are you sure you want to proceed?")
            layout.separator()

            layout.prop(self, "removeExtras")

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self)

class SwitchCC (bpy.types.Operator):
    """Toggles Custom Faces for the rig, so you can use the same clothing textures with a different face texture/decal."""
    bl_label = "Toggle Custom Faces"
    bl_idname = "rrs.togglecc"
    bl_description = "Toggles Custom Faces for the rig, so you can use the same clothing textures with a different face texture/decal."
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        scene = context.scene
        activerrsrig = context.object
        meta1 = activerrsrig.pose.bones.get("rigmeta1")
        meta2 = activerrsrig.pose.bones.get("rigmeta2")
        #
        rrs_materialBLOCK = meta2.get("MatObject")
        rrs_materialNAME = rrs_materialBLOCK.name
        rrs_material = bpy.data.materials.get(rrs_materialNAME)
        rrs_nodeTree = rrs_material.node_tree

        rrs_headBLOCK = meta1.get("headObj")
        rrs_headNAME = rrs_headBLOCK.name
        rrs_head = bpy.data.objects.get(rrs_headNAME)
        #
        mixNode = None

        for node in rrs_nodeTree.nodes:
            if node.name == "RRS-CCSwitcher":
                mixNode = node
                break
        
        if mixNode is None:
            self.report({'ERROR'}, "Couldn't find 'RRS-CCSwitcher' mix node from rig, it is recommended you re-import the rig or manually fix the error.")
            return {'CANCELLED'}

        if meta2["ccFaces"] == True:
            UV_A = rrs_head.data.uv_layers.get("UVMap")
            UV_B = rrs_head.data.uv_layers.get("other") #other here is default
            mixNode.inputs["Factor"].default_value = 1.0
            meta2["ccFaces"] = False

            UV_A.name = "othertemp"
            UV_B.name = "UVMap"
            UV_TEMP = rrs_head.data.uv_layers.get("othertemp")
            UV_TEMP.name = "other"
            self.report({'INFO'}, "Custom Faces successfully disabled.")
            return {'FINISHED'}
        else:
            UV_A = rrs_head.data.uv_layers.get("UVMap")
            UV_B = rrs_head.data.uv_layers.get("other") #other here is faceless
            mixNode.inputs["Factor"].default_value = 0.0
            meta2["ccFaces"] = True

            UV_A.name = "othertemp"
            UV_B.name = "UVMap"
            UV_TEMP = rrs_head.data.uv_layers.get("othertemp")
            UV_TEMP.name = "other"
            self.report({'INFO'}, "Custom Faces successfully enabled on the rig! Change the texture in the Custom Faces sub-panel.")

        return {'FINISHED'}

class CustomFacesPanel (bpy.types.Panel):
    bl_label = "Custom Faces"
    bl_idname = "OBJECT_PT_rrs2CC"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Roblox Rigs Suite'
    bl_parent_id = 'OBJECT_PT_rrs2'
    bl_options = {'DEFAULT_CLOSED'}
    
    def draw (self, context):
        layout = self.layout
        scene = context.scene

        active_obj = context.object

        if active_obj and active_obj.type == 'ARMATURE' and "RRS" in active_obj.name:
            meta2 = active_obj.pose.bones.get("rigmeta2")
            dEligible = meta2.get("driverEligible")
            faceComp = meta2.get("ccFaces")
            #-Material Information-#
            rrs_matvalue = meta2.get("MatObject") #mat value
            rrs_matname = rrs_matvalue.name #mat name
            rrs_charmaterial = bpy.data.materials.get(rrs_matname) #actual mat
            rrs_charnodetree = rrs_charmaterial.node_tree #mat's node tree
            rrs_nodegroup = None #fallback

            imgselectbox = layout.box()

            for node in rrs_charnodetree.nodes: #find the image texture
                if node.type == 'GROUP': #and "customFacesTex_RRS" in node.name:
                    rrs_nodegroup = node.node_tree
                    break

            if faceComp and dEligible and rrs_nodegroup:
                for node in rrs_nodegroup.nodes:
                    if node.type == 'TEX_IMAGE':
                        rrs_CustomFacesNode = node
                        break
                for node in rrs_nodegroup.nodes:
                    if node.type == 'MAPPING':
                        rrs_CustomFacesMapping = node
                        break
                imgselectbox.label(text="Select your face texture:")
                imgselectbox.template_ID(rrs_CustomFacesNode, "image", open="image.open")
                
                if rrs_CustomFacesNode.image: #Image Settings
                    imgselectbox.prop(rrs_CustomFacesNode.image, "source")
                    if rrs_CustomFacesNode.image.source in {'SEQUENCE', 'MOVIE'}:
                        imgselectbox.prop(rrs_CustomFacesNode.image_user, "frame_duration")
                        imgselectbox.prop(rrs_CustomFacesNode.image_user, "frame_start")
                        imgselectbox.prop(rrs_CustomFacesNode.image_user, "frame_offset")
                        imgrow = imgselectbox.row()
                        imgrow.prop(rrs_CustomFacesNode.image_user, "use_cyclic")
                        imgrow.prop(rrs_CustomFacesNode.image_user, "use_auto_refresh")

                imgselectbox.operator("rrs.togglecc", icon='TEXTURE')

                imgmappingbox = layout.box()
                imgmappingbox.prop(scene, "toggleMappingEditUI")
                mapBool = scene.toggleMappingEditUI
                if mapBool:
                    imgmappingbox.prop(rrs_CustomFacesMapping.inputs[1], "default_value", text="Location")
                    imgmappingbox.prop(rrs_CustomFacesMapping.inputs[2], "default_value", text="Rotation")
                    imgmappingbox.prop(rrs_CustomFacesMapping.inputs[3], "default_value", text="Scale")
                else:
                    pass
                
            elif dEligible:
                imgselectbox.operator("rrs.togglecc", icon='TEXTURE')
            elif faceComp and rrs_nodegroup:
                for node in rrs_nodegroup.nodes:
                    if node.type == 'TEX_IMAGE':
                        rrs_CustomFacesNode = node
                        break
                imgselectbox.label(text="Select your face texture:")
                imgselectbox.template_ID(rrs_CustomFacesNode, "image", open="image.open")
                imgselectboxCol = imgselectbox.column()
                imgselectboxCol.operator("rrs.togglecc", icon='TEXTURE')
                imgselectboxCol.enabled = False
            else:
                imgselectbox.label(text="Rig does not have addon compatibility!", icon='CANCEL')
                pass
        else:
            pass

#I don't really like that I'm adding this to the custom faces panel but here goes

class MaterialsAdanced (bpy.types.Panel):
    bl_label = "Advanced"
    bl_idname = "OBJECT_PT_rrs2adv"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Roblox Rigs Suite'
    bl_parent_id = 'OBJECT_PT_rrs2'
    bl_options = {'DEFAULT_CLOSED'}
    
    def draw (self, context):
        layout = self.layout
        scene = context.scene

        active_obj = context.object

        if active_obj and active_obj.type == 'ARMATURE' and "RRS" in active_obj.name:
            meta2 = active_obj.pose.bones.get("rigmeta2")
            dEligible = meta2.get("driverEligible")

            if dEligible is False:
                col = layout.column()
                col.operator("rrs.removematerialcompatibility", icon='X')
                col.enabled = False
            elif dEligible is True:
                col = layout.column()
                col.operator("rrs.removematerialcompatibility", icon='X')
            else:
                pass
        else:
            pass

classes = [BasicMaterials, SwitchCC, CustomFacesPanel, MaterialsAdanced]


def register():
    for cls in (classes):
        bpy.utils.register_class(cls)
    
    bpy.types.Scene.toggleMappingEditUI = bpy.props.BoolProperty(
        name="Edit Mapping",
        description="Allows you to edit the mapping of the custom face texture.",
        default=False
    )

def unregister():
    for cls in (classes):
        bpy.utils.unregister_class(cls)
    del bpy.types.Scene.toggleMappingEditUI

if __name__ == "__main__":
    register()