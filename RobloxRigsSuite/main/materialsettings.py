import bpy

class TextureInfo (bpy.types.Operator):
    """Opens a window that guides the user on why their texture might be broken."""
    bl_label = "Broken Texture?"
    bl_idname = "rrs.textureinfo"
    bl_description = "Opens a window that guides the user on why their texture might be broken."

    def execute(self, context):
        pass
        return {'FINISHED'}

    def draw(self, context):
        layout = self.layout
        scene = context.scene

        layout.label(text="Is your R15 texture broken? You might want to change")
        layout.label(text="the rig variant in Importer > Advanced > Variant.")

    def invoke(self, context, event):
        return context.window_manager.invoke_popup(self)

class MaterialSettingsPanel (bpy.types.Panel):
    bl_label = "Material Settings"
    bl_idname = "OBJECT_PT_rrs2"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Roblox Rigs Suite'
    bl_options = {'DEFAULT_CLOSED'}

    def draw (self, context):
        layout = self.layout
        scene = context.scene

        active_obj = context.object

        if active_obj and "RRS" in active_obj.name and active_obj.type == 'ARMATURE':
            meta1 = active_obj.pose.bones.get("rigmeta1")
            meta2 = active_obj.pose.bones.get("rigmeta2")
            rigID = meta1["rigID"]
            #-Material Information-#
            rrs_matvalue = meta2.get("MatObject") #mat value
            rrs_matname = rrs_matvalue.name #mat name
            rrs_charmaterial = bpy.data.materials.get(rrs_matname) #actual mat
            rrs_charnodetree = rrs_charmaterial.node_tree #mat's node tree

            dEligible = meta2["driverEligible"]

            imgselectbox = layout.box()
            #
            for node in rrs_charnodetree.nodes: #find the image texture
                if node.type == 'TEX_IMAGE':
                    rrs_imagetexturenode = node
                    break

            if rrs_imagetexturenode: #if its found it then display it's selector.
                imgselectbox.label(text="Select your texture:")
                imgselectbox.template_ID(rrs_imagetexturenode, "image", open="image.open")
                if "g2" in rigID and rrs_imagetexturenode.image:
                    imgselectbox.operator("rrs.textureinfo", icon='QUESTION')

            #
            matCol = layout.column()
            if dEligible == False:
                matCol.enabled = False
                matCol.prop (meta2, '["Apply Bump"]', text="Apply Bump")
                matCol.prop (meta2, '["Glossy"]', text="Glossy")
            elif dEligible == True:
                matCol.prop (meta2, '["Apply Bump"]', text="Apply Bump")
                matCol.prop (meta2, '["Glossy"]', text="Glossy")
            else:
                pass

        else:
            layout.label(text="No rig selected.", icon='CANCEL')
            pass
        
        #-View mode warning-#    
        shading_mode = None
        for area in context.screen.areas:
            if area.type == 'VIEW_3D':  # Look for the 3D Viewport
                shading_mode = area.spaces.active.shading.type
                break

        if shading_mode in {'WIREFRAME', 'SOLID'}:
            layout.label (text="Changes may not be visible.", icon= 'WARNING_LARGE')
            
classes = [TextureInfo, MaterialSettingsPanel]

def register():
    for cls in (classes):
        bpy.utils.register_class(cls)

def unregister():
    for cls in (classes):
        bpy.utils.unregister_class(cls)

if __name__ == "__main__":
    register()