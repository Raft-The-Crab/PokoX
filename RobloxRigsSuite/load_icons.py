import bpy
import os
import bpy.utils.previews
from . import __package__ as addonKey

preview_collections = {}
prefs = bpy.context.preferences.addons[addonKey].preferences

def load_ico():
    #if not getattr(prefs, "customIco", True):
    #    return

    pcoll = bpy.utils.previews.new()
    ico = os.path.join(os.path.dirname(__file__), "rrs_ico")

    pcoll.load("rrs_logo", os.path.join(ico, "rrs_logo.png"), 'IMAGE')
    pcoll.load("rrs1", os.path.join(ico, "rrs1.png"), 'IMAGE')
    pcoll.load("rrs2", os.path.join(ico, "rrs2.png"), 'IMAGE')
    pcoll.load("rrs3", os.path.join(ico, "rrs3.png"), 'IMAGE')
    pcoll.load("rrs4", os.path.join(ico, "rrs4.png"), 'IMAGE')
    pcoll.load("rrs5", os.path.join(ico, "rrs5.png"), 'IMAGE')
    pcoll.load("rrs6", os.path.join(ico, "rrs6.png"), 'IMAGE')
    pcoll.load("rrs7", os.path.join(ico, "rrs7.png"), 'IMAGE')

    preview_collections["rrs_icons"] = pcoll


def del_ico():
    for pcoll in preview_collections.values():
        bpy.utils.previews.remove(pcoll)
    preview_collections.clear()

def get_ico(name):
    pcoll = preview_collections.get("rrs_icons")
    if pcoll and name in pcoll:
        return pcoll[name].icon_id
    return 0