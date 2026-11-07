import bpy
from . import compatibility_remover
from ... import __package__ as addonKey

class AdvancedPanel (bpy.types.Panel):
    bl_label = "Advanced"
    bl_idname = "OBJECT_PT_rrstools_Advanced"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Roblox Rigs Suite'
    bl_parent_id = "OBJECT_PT_rrs4"
    bl_options = {'DEFAULT_CLOSED'}

    @classmethod
    def poll (self, context):
        prefs = bpy.context.preferences.addons[addonKey].preferences
        return prefs.advancedTools

    def draw (self, context):
        layout = self.layout
        scene = context.scene
        active_obj = context.object
        
        if active_obj:
            layout.operator("rrs.tools_removecompatibility", icon='X')
        else:
            pass

classes = [AdvancedPanel]


def register():
    for cls in (classes):
        bpy.utils.register_class(cls)
    compatibility_remover.register()

def unregister():
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
    compatibility_remover.unregister()
    
if __name__ == "__main__":
    register()