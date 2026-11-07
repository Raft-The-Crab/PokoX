import bpy
import os
from ..load_icons import get_ico
from .. import __package__ as addonKey

rrs_relative = os.path.dirname(os.path.abspath(__file__))
rrs_root = os.path.dirname(rrs_relative)
rrs_path = os.path.join(rrs_root, "rigs")
rrs_directory = os.path.normpath(rrs_path)

#-R6 Rig Operators
class RRS1G1 (bpy.types.Operator):
    """Imports a Roblox Rigs Suite rig."""
    bl_idname = "rrs.import_rrs1g1"
    bl_label = "Import Blocky R6"
    bl_description = "Imports a Roblox Rigs Suite rig."
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        rig_file = os.path.join(rrs_directory, "BlockyR6.blend")
        collection = "Blocky R6 (RRS Rig)"

        with bpy.data.libraries.load(rig_file, link=False) as (data_from, data_to):
            data_to.collections = [collection]
        context.collection.children.link(data_to.collections[0])

        print ('Roblox Rigs Suite: Rig imported.')
        self.report({'INFO'}, "Rig has been imported!")

        return{'FINISHED'}

class RRS2G1 (bpy.types.Operator):
    """Imports a Roblox Rigs Suite rig."""
    bl_idname = "rrs.import_rrs2g1"
    bl_label = "Import Robloxian 2.0 R6"
    bl_description = "Imports a Roblox Rigs Suite rig."
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        rig_file = os.path.join(rrs_directory, "Robloxian_2.0_Mesh_R6.blend")
        collection = "Robloxian 2.0 R6 (RRS Rig)"

        with bpy.data.libraries.load(rig_file, link=False) as (data_from, data_to):
            data_to.collections = [collection]
        context.collection.children.link(data_to.collections[0])

        print ('Roblox Rigs Suite: Rig imported.')
        self.report({'INFO'}, "Rig has been imported!")

        return{'FINISHED'}

class RRS3G1 (bpy.types.Operator):
    """Imports a Roblox Rigs Suite rig."""
    bl_idname = "rrs.import_rrs3g1"
    bl_label = "Import Man R6"
    bl_description = "Imports a Roblox Rigs Suite rig."
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        rig_file = os.path.join(rrs_directory, "ManR6.blend")
        collection = "Man R6 (RRS Rig)"

        with bpy.data.libraries.load(rig_file, link=False) as (data_from, data_to):
            data_to.collections = [collection]
        context.collection.children.link(data_to.collections[0])

        print ('Roblox Rigs Suite: Rig imported.')
        self.report({'INFO'}, "Rig has been imported!")

        return{'FINISHED'}

class RRS4G1 (bpy.types.Operator):
    """Imports a Roblox Rigs Suite rig."""
    bl_idname = "rrs.import_rrs4g1"
    bl_label = "Import Woman R6"
    bl_description = "Imports a Roblox Rigs Suite rig."
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        rig_file = os.path.join(rrs_directory, "WomanR6.blend")
        collection = "Woman R6 (RRS Rig)"

        with bpy.data.libraries.load(rig_file, link=False) as (data_from, data_to):
            data_to.collections = [collection]
        context.collection.children.link(data_to.collections[0])

        print ('Roblox Rigs Suite: Rig imported.')
        self.report({'INFO'}, "Rig has been imported!")

        return{'FINISHED'}

class RRS5G1 (bpy.types.Operator):
    """Imports a Roblox Rigs Suite rig."""
    bl_idname = "rrs.import_rrs5g1"
    bl_label = "Import Boy R6"
    bl_description = "Imports a Roblox Rigs Suite rig."
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        rig_file = os.path.join(rrs_directory, "BoyR6.blend")
        collection = "Boy R6 (RRS Rig)"

        with bpy.data.libraries.load(rig_file, link=False) as (data_from, data_to):
            data_to.collections = [collection]
        context.collection.children.link(data_to.collections[0])

        print ('Roblox Rigs Suite: Rig imported.')
        self.report({'INFO'}, "Rig has been imported!")

        return{'FINISHED'}

class RRS6G1 (bpy.types.Operator):
    """Imports a Roblox Rigs Suite rig."""
    bl_idname = "rrs.import_rrs6g1"
    bl_label = "Import Girl R6"
    bl_description = "Imports a Roblox Rigs Suite rig."
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        rig_file = os.path.join(rrs_directory, "GirlR6.blend")
        collection = "Girl R6 (RRS Rig)"

        with bpy.data.libraries.load(rig_file, link=False) as (data_from, data_to):
            data_to.collections = [collection]
        context.collection.children.link(data_to.collections[0])

        print ('Roblox Rigs Suite: Rig imported.')
        self.report({'INFO'}, "Rig has been imported!")

        return{'FINISHED'}

class RRS7G1 (bpy.types.Operator):
    """Imports a Roblox Rigs Suite rig."""
    bl_idname = "rrs.import_rrs7g1"
    bl_label = "Import Superhero R6"
    bl_description = "Imports a Roblox Rigs Suite rig."
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        rig_file = os.path.join(rrs_directory, "SuperheroR6.blend")
        collection = "Superhero R6 (RRS Rig)"

        with bpy.data.libraries.load(rig_file, link=False) as (data_from, data_to):
            data_to.collections = [collection]
        context.collection.children.link(data_to.collections[0])

        print ('Roblox Rigs Suite: Rig imported.')
        self.report({'INFO'}, "Rig has been imported!")

        return{'FINISHED'}

#-R15 Rig Operators
class RRS1G2 (bpy.types.Operator):
    """Imports a Roblox Rigs Suite rig."""
    bl_idname = "rrs.import_rrs1g2"
    bl_label = "Import Blocky R15"
    bl_description = "Imports a Roblox Rigs Suite rig."
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        rig_file = os.path.join(rrs_directory, "BlockyR15.blend")
        collection = "Blocky R15 (RRS Rig)"

        with bpy.data.libraries.load(rig_file, link=False) as (data_from, data_to):
            data_to.collections = [collection]
        context.collection.children.link(data_to.collections[0])

        print ('Roblox Rigs Suite: Rig imported.')
        self.report({'INFO'}, "Rig has been imported!")

        return{'FINISHED'}

class RRS2G2 (bpy.types.Operator):
    """Imports a Roblox Rigs Suite rig."""
    bl_idname = "rrs.import_rrs2g2"
    bl_label = "Import Robloxian 2.0 R15"
    bl_description = "Imports a Roblox Rigs Suite rig."
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        rig_file = os.path.join(rrs_directory, "Robloxian_2.0_Mesh_R15.blend")
        collection = "Robloxian 2.0 R15 (RRS Rig)"

        with bpy.data.libraries.load(rig_file, link=False) as (data_from, data_to):
            data_to.collections = [collection]
        context.collection.children.link(data_to.collections[0])

        print ('Roblox Rigs Suite: Rig imported.')
        self.report({'INFO'}, "Rig has been imported!")

        return{'FINISHED'}

class RRS3G2 (bpy.types.Operator):
    """Imports a Roblox Rigs Suite rig."""
    bl_idname = "rrs.import_rrs3g2"
    bl_label = "Import Man R15"
    bl_description = "Imports a Roblox Rigs Suite rig."
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        rig_file = os.path.join(rrs_directory, "ManR15.blend")
        collection = "Man R15 (RRS Rig)"

        with bpy.data.libraries.load(rig_file, link=False) as (data_from, data_to):
            data_to.collections = [collection]
        context.collection.children.link(data_to.collections[0])

        print ('Roblox Rigs Suite: Rig imported.')
        self.report({'INFO'}, "Rig has been imported!")

        return{'FINISHED'}

class RRS4G2 (bpy.types.Operator):
    """Imports a Roblox Rigs Suite rig."""
    bl_idname = "rrs.import_rrs4g2"
    bl_label = "Import Woman R15"
    bl_description = "Imports a Roblox Rigs Suite rig."
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        rig_file = os.path.join(rrs_directory, "WomanR15.blend")
        collection = "Woman R15 (RRS Rig)"

        with bpy.data.libraries.load(rig_file, link=False) as (data_from, data_to):
            data_to.collections = [collection]
        context.collection.children.link(data_to.collections[0])

        print ('Roblox Rigs Suite: Rig imported.')
        self.report({'INFO'}, "Rig has been imported!")

        return{'FINISHED'}

class RRS5G2 (bpy.types.Operator):
    """Imports a Roblox Rigs Suite rig."""
    bl_idname = "rrs.import_rrs5g2"
    bl_label = "Import Boy R15"
    bl_description = "Imports a Roblox Rigs Suite rig."
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        rig_file = os.path.join(rrs_directory, "BoyR15.blend")
        collection = "Boy R15 (RRS Rig)"

        with bpy.data.libraries.load(rig_file, link=False) as (data_from, data_to):
            data_to.collections = [collection]
        context.collection.children.link(data_to.collections[0])

        print ('Roblox Rigs Suite: Rig imported.')
        self.report({'INFO'}, "Rig has been imported!")

        return{'FINISHED'}

class RRS6G2 (bpy.types.Operator):
    """Imports a Roblox Rigs Suite rig."""
    bl_idname = "rrs.import_rrs6g2"
    bl_label = "Import Girl R15"
    bl_description = "Imports a Roblox Rigs Suite rig."
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        rig_file = os.path.join(rrs_directory, "GirlR15.blend")
        collection = "Girl R15 (RRS Rig)"

        with bpy.data.libraries.load(rig_file, link=False) as (data_from, data_to):
            data_to.collections = [collection]
        context.collection.children.link(data_to.collections[0])

        print ('Roblox Rigs Suite: Rig imported.')
        self.report({'INFO'}, "Rig has been imported!")

        return{'FINISHED'}

class RRS7G2 (bpy.types.Operator):
    """Imports a Roblox Rigs Suite rig."""
    bl_idname = "rrs.import_rrs7g2"
    bl_label = "Import Superhero R15"
    bl_description = "Imports a Roblox Rigs Suite rig."
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        rig_file = os.path.join(rrs_directory, "SuperheroR15.blend")
        collection = "Superhero R15 (RRS Rig)"

        with bpy.data.libraries.load(rig_file, link=False) as (data_from, data_to):
            data_to.collections = [collection]
        context.collection.children.link(data_to.collections[0])

        print ('Roblox Rigs Suite: Rig imported.')
        self.report({'INFO'}, "Rig has been imported!")

        return{'FINISHED'}

#-Menus
class ImportRRS_Menu (bpy.types.Menu):
    bl_label = "Roblox Rigs Suite"
    bl_idname = "VIEW3D_MT_RRS_Imp1"

    @classmethod
    def poll(cls, context):
        prefs = bpy.context.preferences.addons[addonKey].preferences
        return prefs.toggleSecondaryImporter

    def draw (self, context):
        layout = self.layout
    
def rrs_menu1(self, context):
    self.layout.menu("VIEW3D_MT_RRS_Imp1", icon_value=get_ico("rrs_logo"))

class ImportRRS_R6 (bpy.types.Menu):
    bl_label = "R6 Rigs"
    bl_idname = "VIEW3D_MT_RRS_Imp2"
    
    @classmethod
    def poll(cls, context):
        prefs = bpy.context.preferences.addons[addonKey].preferences
        return prefs.toggleSecondaryImporter

    def draw (self, context):
        layout = self.layout
        scene = context.scene

        layout.operator("rrs.import_rrs1g1", text="Blocky Rig", icon_value=get_ico("rrs1"))
        layout.operator("rrs.import_rrs2g1", text="Robloxian 2.0 Rig", icon_value=get_ico("rrs2"))
        layout.operator("rrs.import_rrs3g1", text="Man Rig", icon_value=get_ico("rrs3"))
        layout.operator("rrs.import_rrs4g1", text="Woman Rig", icon_value=get_ico("rrs4"))
        layout.operator("rrs.import_rrs5g1", text="Boy Rig", icon_value=get_ico("rrs5"))
        layout.operator("rrs.import_rrs6g1", text="Girl Rig", icon_value=get_ico("rrs6"))
        layout.operator("rrs.import_rrs7g1", text="Superhero Rig", icon_value=get_ico("rrs7"))
    
def rrs_menu2(self, context):
    self.layout.menu("VIEW3D_MT_RRS_Imp2")

class ImportRRS_R15 (bpy.types.Menu):
    bl_label = "R15 Rigs"
    bl_idname = "VIEW3D_MT_RRS_Imp3"

    @classmethod
    def poll(cls, context):
        prefs = bpy.context.preferences.addons[addonKey].preferences
        return prefs.toggleSecondaryImporter

    def draw (self, context):
        layout = self.layout
        scene = context.scene

        layout.operator("rrs.import_rrs1g2", text="Blocky Rig", icon_value=get_ico("rrs1"))
        layout.operator("rrs.import_rrs2g2", text="Robloxian 2.0 Rig", icon_value=get_ico("rrs2"))
        layout.operator("rrs.import_rrs3g2", text="Man Rig", icon_value=get_ico("rrs3"))
        layout.operator("rrs.import_rrs4g2", text="Woman Rig", icon_value=get_ico("rrs4"))
        layout.operator("rrs.import_rrs5g2", text="Boy Rig", icon_value=get_ico("rrs5"))
        layout.operator("rrs.import_rrs6g2", text="Girl Rig", icon_value=get_ico("rrs6"))
        layout.operator("rrs.import_rrs7g2", text="Superhero Rig", icon_value=get_ico("rrs7"))
    
def rrs_menu3(self, context):
    self.layout.menu("VIEW3D_MT_RRS_Imp3")

#-Register
classes = classes = [
    RRS1G1, RRS2G1, RRS3G1, RRS4G1, RRS5G1, RRS6G1, RRS7G1, 
    RRS1G2, RRS2G2, RRS3G2, RRS4G2, RRS5G2, RRS6G2, RRS7G2, 
    ImportRRS_Menu, ImportRRS_R6, ImportRRS_R15
]

def register():
    for cls in (classes):
        bpy.utils.register_class(cls)
    bpy.types.VIEW3D_MT_add.append(rrs_menu1)
    bpy.types.VIEW3D_MT_RRS_Imp1.append(rrs_menu2)
    bpy.types.VIEW3D_MT_RRS_Imp1.append(rrs_menu3)

def unregister():
    bpy.types.VIEW3D_MT_RRS_Imp1.remove(rrs_menu2)
    bpy.types.VIEW3D_MT_RRS_Imp1.remove(rrs_menu3)
    bpy.types.VIEW3D_MT_add.remove(rrs_menu1)
    for cls in (classes):
        bpy.utils.unregister_class(cls)

if __name__ == "__main__":
    register()