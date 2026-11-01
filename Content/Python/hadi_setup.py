"""HADI: VICE CITY - generates the project's master materials.

The whole city, Hadi and the vehicles are built from engine primitives at runtime, so the only
assets the game needs are three parameterised master materials. They are created here with the
MaterialEditingLibrary the first time the project is opened (or manually via
Tools > Execute Python Script... > Content/Python/hadi_setup.py).

Parameter names must match Source/HadiViceCity/Private/World/HadiMaterialSubsystem.cpp.
"""
import unreal

MATERIAL_DIR = "/Game/Hadi/Materials"
MEL = unreal.MaterialEditingLibrary

WATER_NORMAL_HLSL = """
float2 p = UV;
float2 d = float2(0.0, 0.0);
const float2 dir1 = float2(0.8, 0.6);
const float2 dir2 = float2(-0.4, 0.9165);
const float2 dir3 = float2(0.3, -0.954);
d += dir1 * 0.10 * cos(dot(p, dir1) * 6.2832 + T * 1.1);
d += dir2 * 0.07 * cos(dot(p, dir2) * 10.681 + T * 1.6);
d += dir3 * 0.05 * cos(dot(p, dir3) * 19.478 + T * 2.3);
return normalize(float3(-d.x, -d.y, 1.0));
"""


def _log(message):
    unreal.log("[Hadi] " + message)


def _new_material(name):
    path = "{}/{}".format(MATERIAL_DIR, name)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return None
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = tools.create_asset(name, MATERIAL_DIR, unreal.Material, unreal.MaterialFactoryNew())
    material.set_editor_property("used_with_instanced_static_meshes", True)
    return material


def _vector(material, name, default, x, y):
    node = MEL.create_material_expression(material, unreal.MaterialExpressionVectorParameter, x, y)
    node.set_editor_property("parameter_name", name)
    node.set_editor_property("default_value", unreal.LinearColor(*default))
    return node


def _scalar(material, name, default, x, y):
    node = MEL.create_material_expression(material, unreal.MaterialExpressionScalarParameter, x, y)
    node.set_editor_property("parameter_name", name)
    node.set_editor_property("default_value", default)
    return node


def _multiply(material, a, b, x, y):
    node = MEL.create_material_expression(material, unreal.MaterialExpressionMultiply, x, y)
    MEL.connect_material_expressions(a, "", node, "A")
    MEL.connect_material_expressions(b, "", node, "B")
    return node


def _finish(material):
    MEL.layout_material_expressions(material)
    MEL.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    _log("created " + material.get_path_name())


def build_opaque():
    """M_Hadi_Opaque: every painted / concrete / glass / neon surface in the city."""
    material = _new_material("M_Hadi_Opaque")
    if not material:
        return
    base = _vector(material, "BaseColor", (0.8, 0.8, 0.8, 1.0), -700, -300)
    roughness = _scalar(material, "Roughness", 0.6, -700, -100)
    metallic = _scalar(material, "Metallic", 0.0, -700, 0)
    specular = _scalar(material, "Specular", 0.5, -700, 100)
    emissive_color = _vector(material, "EmissiveColor", (0.0, 0.0, 0.0, 1.0), -900, 250)
    emissive_strength = _scalar(material, "EmissiveStrength", 0.0, -900, 450)
    emissive = _multiply(material, emissive_color, emissive_strength, -500, 300)
    MEL.connect_material_property(base, "", unreal.MaterialProperty.MP_BASE_COLOR)
    MEL.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    MEL.connect_material_property(metallic, "", unreal.MaterialProperty.MP_METALLIC)
    MEL.connect_material_property(specular, "", unreal.MaterialProperty.MP_SPECULAR)
    MEL.connect_material_property(emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    _finish(material)


def build_glass():
    """M_Hadi_Glass: see-through glass (vehicle windows, waypoint column)."""
    material = _new_material("M_Hadi_Glass")
    if not material:
        return
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    material.set_editor_property("translucency_lighting_mode", unreal.TranslucencyLightingMode.TLM_SURFACE)
    material.set_editor_property("two_sided", True)
    base = _vector(material, "BaseColor", (0.1, 0.14, 0.18, 1.0), -600, -200)
    opacity = _scalar(material, "Opacity", 0.32, -600, 0)
    roughness = _scalar(material, "Roughness", 0.03, -600, 100)
    MEL.connect_material_property(base, "", unreal.MaterialProperty.MP_BASE_COLOR)
    MEL.connect_material_property(opacity, "", unreal.MaterialProperty.MP_OPACITY)
    MEL.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    _finish(material)


def build_water():
    """M_Hadi_Water: ocean, pools and fountains with animated procedural wave normals."""
    material = _new_material("M_Hadi_Water")
    if not material:
        return
    material.set_editor_property("tangent_space_normal", True)
    base = _vector(material, "BaseColor", (0.01, 0.12, 0.18, 1.0), -600, -300)
    roughness = _scalar(material, "Roughness", 0.04, -600, -150)
    specular = _scalar(material, "Specular", 0.8, -600, -50)
    tiling = _scalar(material, "Tiling", 4.0, -1100, 250)
    texcoord = MEL.create_material_expression(material, unreal.MaterialExpressionTextureCoordinate, -1100, 100)
    uv = _multiply(material, texcoord, tiling, -900, 150)
    time = MEL.create_material_expression(material, unreal.MaterialExpressionTime, -900, 350)
    custom = MEL.create_material_expression(material, unreal.MaterialExpressionCustom, -600, 200)
    custom.set_editor_property("code", WATER_NORMAL_HLSL)
    custom.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    custom.set_editor_property("description", "HadiWaterNormal")
    uv_input = unreal.CustomInput()
    uv_input.set_editor_property("input_name", "UV")
    time_input = unreal.CustomInput()
    time_input.set_editor_property("input_name", "T")
    custom.set_editor_property("inputs", [uv_input, time_input])
    MEL.connect_material_expressions(uv, "", custom, "UV")
    MEL.connect_material_expressions(time, "", custom, "T")
    MEL.connect_material_property(base, "", unreal.MaterialProperty.MP_BASE_COLOR)
    MEL.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    MEL.connect_material_property(specular, "", unreal.MaterialProperty.MP_SPECULAR)
    MEL.connect_material_property(custom, "", unreal.MaterialProperty.MP_NORMAL)
    _finish(material)


def ensure_assets():
    if not unreal.EditorAssetLibrary.does_directory_exist(MATERIAL_DIR):
        unreal.EditorAssetLibrary.make_directory(MATERIAL_DIR)
    build_opaque()
    build_glass()
    build_water()
    _log("materials ready in " + MATERIAL_DIR)


if __name__ == "__main__":
    ensure_assets()
