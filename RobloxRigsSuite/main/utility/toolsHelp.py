import bpy
from ... import __package__ as addonKey

class RRSToolsHelp (bpy.types.Panel):
    bl_label = "Help"
    bl_idname = "OBJECT_PT_rrs4h"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Roblox Rigs Suite'
    bl_parent_id = "OBJECT_PT_rrs4"
    bl_options = {'DEFAULT_CLOSED'}

    @classmethod
    def poll(cls, context):
        prefs = bpy.context.preferences.addons[addonKey].preferences
        return prefs.toolsHelp

    def draw (self, context):
        layout = self.layout
        scene = context.scene
        
        layout.label(text="Not enough tools?", icon='QUESTION')
        layout.label(text="Enable/disable tools in the addon preferences.")
        layout.operator("screen.userpref_show", text="Open Preferences", icon='PREFERENCES')

classes = [RRSToolsHelp]

def register():
    for cls in (classes):
        bpy.utils.register_class(cls)

def unregister():
    for cls in (classes):
        bpy.utils.unregister_class(cls)

if __name__ == "__main__":
    register()