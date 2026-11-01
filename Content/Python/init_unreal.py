"""HADI: VICE CITY - editor bootstrap.

Unreal runs every init_unreal.py found on the Python path when the editor starts. We wait until
the asset registry has finished its initial scan and then make sure the project's generated
materials exist (see hadi_setup.py). Nothing runs in packaged builds.
"""
import unreal

_tick_handle = None


def _on_tick(_delta_seconds):
    global _tick_handle
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    if registry.is_loading_assets():
        return
    unreal.unregister_slate_post_tick_callback(_tick_handle)
    _tick_handle = None
    try:
        import hadi_setup

        hadi_setup.ensure_assets()
    except Exception as error:  # keep the editor usable even if generation fails
        unreal.log_error("[Hadi] Material generation failed: {}".format(error))


_tick_handle = unreal.register_slate_post_tick_callback(_on_tick)
