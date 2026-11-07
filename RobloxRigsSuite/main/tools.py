import bpy
from .utility import attach_custom_heads
from .utility import clothing_3d
from .utility import attach_accessories
from .utility import animation_linker
from .utility import advanced_tools
from .utility import toolsHelp

class RRSToolsPanel (bpy.types.Panel):
    bl_label = "Tools"
    bl_idname = "OBJECT_PT_rrs4"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Roblox Rigs Suite'
    bl_options = {'DEFAULT_CLOSED'}

    def draw (self, context):
        layout = self.layout
        scene = context.scene

classes = [RRSToolsPanel]

def safe_unregister(module):
    try:
        module.unregister()
    except Exception:
        pass

def register():
    #rrsprefs = bpy.context.preferences.addons["RobloxRigsSuite - Development Edition"].preferences

    for cls in (classes):
        bpy.utils.register_class(cls)
    
    attach_custom_heads.register()
    clothing_3d.register()
    attach_accessories.register()
    animation_linker.register()
    advanced_tools.register()
    toolsHelp.register()

    #if rrsprefs.attachCustomHeads:
    #    attach_custom_heads.register()
    #if rrsprefs.layeredClothing:
    #    clothing_3d.register()
    #if rrsprefs.attachAccessories:
    #    attach_accessories.register()
    #if rrsprefs.animationLink:
    #    animation_linker.register()
    #
    #if rrsprefs.toolsHelp:
    #    toolsHelp.register()

def unregister():
    for cls in (classes):
        bpy.utils.unregister_class(cls)

    toolsHelp.unregister()
    advanced_tools.unregister()
    animation_linker.unregister()
    attach_accessories.unregister()
    clothing_3d.unregister()
    attach_custom_heads.unregister()

    #safe_unregister(toolsHelp)
#
    #safe_unregister(animation_linker)
    #safe_unregister(attach_accessories)
    #safe_unregister(clothing_3d)
    #safe_unregister(attach_custom_heads)
    

if __name__ == "__main__":
    register()