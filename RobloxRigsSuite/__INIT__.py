bl_info = {
    "name": "Roblox Rigs Suite",
    "description": "RRS lets you quickly import Roblox rigs and customize them with extra tools and quick settings to get you started",
    "author": "Comix Productions",
    "version": (1, 2, 2),
    "blender": (5, 0, 0),
    "location": "View3D > Roblox Rigs Suite",
    "warning": "", # used for warning icon and text in addons panel
    "doc_url": "",
    "tracker_url": "",
    "support": "COMMUNITY",
    "category": "Rigging",
}

import bpy
from . import load_icons
from .main import importer
from .main import import_menu
from .main import rigsettings
from .main import iksettings
from .main import materialsettings
from .main import tools
from .main import customFaces
from . import about

class RRSPreferences (bpy.types.AddonPreferences):
    bl_idname = __name__

    #def update_modules(self, context):
    #    unregister()
    #    register()

    #def update_icons(self, context):
    #    from . import load_icons
    #    load_icons.del_ico()
    #    load_icons.load_ico()
    
    toggleSecondaryImporter: bpy.props.BoolProperty(
        name="Add Menu Importer",
        description="Allows you to import rigs from the Add menu in the 3D viewport.",
        default=True
    )

    rigManager: bpy.props.BoolProperty(
        name="Show Rig Manager",
        description="Enables the rig manager, which is located in the Rig Settings panel.",
        default=True
    )

    unlinkRigs: bpy.props.BoolProperty(
        name="Unlink Rigs",
        description="Unlinks rigs when you delete them, so that they are fully gone from the Blender file.",
        default=False
    )

    #customIco: bpy.props.BoolProperty(
    #    name="Show Custom Icons (Requires Restart)",
    #    description="Shows custom icons in certain parts of the UI.",
    #    default=True,
    #    update=update_icons
    #)

    #deselectImport: bpy.props.EnumProperty(
    #    name="Import Deselection",
    #    description="Deselects objects when importing rigs.",
    #    items=[('DE_1', "Deselect Before and After Importing", ""),
    #           ('DE_2', "Deselect Before Importing", ""),
    #           ('DE_3', "Deselect After Importing", ""),
    #           ('DE_4', "Don't Deselect", ""),],
    #)

    #featureSet = bpy.props.EnumProperty(
    #    name="Feature Set",
    #    description="Changes the type of features that show in the RRS addon.",
    #    items=[('RRS_0', "Full Add-on", "Includes all features that come with RRS."),
    #           ('RRS_1', "Simple Mode", "Will load a stripped back version of RRS."),
    #    ],
    #    default='RRS_0',
    #)

#-----------------------------------------------------------------

    attachCustomHeads: bpy.props.BoolProperty(
        name="Attach Custom Heads",
        description="Enables the 'Attach Custom Heads' tool.",
        default=True,
        #update=update_modules
    )
    
    layeredClothing: bpy.props.BoolProperty(
        name="3D/Layered Clothing",
        description="Enables the '3D/Layered Clothing' tool.",
        default=True,
        #update=update_modules
    )

    attachAccessories: bpy.props.BoolProperty(
        name="Attach Accessories",
        description="Enables the 'Attach Accessories' tool.",
        default=True,
        #update=update_modules
    )

    animationLink: bpy.props.BoolProperty(
        name="Animation Linker",
        description="Enables the 'Animation Linker' tool.",
        default=True,
        #update=update_modules
    )

    advancedTools: bpy.props.BoolProperty(
        name="Advanced Tools",
        description="Enables advanced tools.",
        default=False,
        #update=update_modules
    )

    toolsHelp: bpy.props.BoolProperty(
        name="Help Panel",
        description="Enables the help panel.",
        default=True,
        #update=update_modules
    )

#-----------------------------------------------------------------
    
    devMode: bpy.props.BoolProperty(
        name="Developer Mode",
        description="Developer mode enables developers to test the addon with extra tools useful for development.",
        default=False
    )

    debugMode: bpy.props.BoolProperty(
        name="Enable Debug",
        description="Will print more logs on certain functions and operations to help debug the addon.",
        default=False
    )



    def draw(self, context):
        layout = self.layout
        
        #General Options
        GenBox = layout.box()
        GenBox.label(text="General Options:", icon='SETTINGS')
        GenBox.separator()
        #GenBox.prop(self, "featureSet")
        GenBox.prop(self, "unlinkRigs")
        #GenBox.prop(self, "customIco")
        GenRow = GenBox.row()
        GenRow.prop(self, "toggleSecondaryImporter")
        GenRow.prop(self, "rigManager")

        #Tools Options
        ToolsBox = layout.box()
        ToolsBox.label(text="Enable/Disable Tools:", icon='MODIFIER')
        ToolsBox.separator()
        ToolsBox.prop(self, "attachCustomHeads")
        ToolsBox.prop(self, "layeredClothing")
        ToolsBox.prop(self, "attachAccessories")
        ToolsBox.prop(self, "animationLink")
        ToolsRow = ToolsBox.row()
        ToolsRow.prop(self, "advancedTools")
        ToolsRow.prop(self, "toolsHelp")

        #Advanced Options
        advBox = layout.box()
        advBox.label(text="Advanced Options:", icon='SETTINGS')
        advBox.separator()
        advRow = advBox.row()
        advRow.prop(self, "debugMode")

rrv_addon = bpy.context.preferences.addons.get(__name__)

classes = [RRSPreferences]

def register():
    for cls in classes:
        bpy.utils.register_class(cls)
    load_icons.load_ico()
    importer.register()
    import_menu.register()
    rigsettings.register()
    iksettings.register()
    materialsettings.register()
    customFaces.register()
    tools.register()
    about.register()

def unregister():
    about.unregister()
    tools.unregister()
    customFaces.unregister()
    materialsettings.unregister()
    iksettings.unregister()
    rigsettings.unregister()
    import_menu.unregister()
    importer.unregister()
    #load_icons.del_ico()
    for cls in classes:
        bpy.utils.unregister_class(cls)

if __name__ == "__main__":
    register()