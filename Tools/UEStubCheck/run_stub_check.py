#!/usr/bin/env python3
"""HADI: VICE CITY - offline compile check for the Unreal layer.

Without the engine we can't run UnrealBuildTool, so this script:
  1. copies Source/HadiViceCity to a temp dir and replaces UHT's GENERATED_BODY() with a macro
     that defines Super / ThisClass / StaticClass the way the generated headers would,
  2. points the compiler at small stub headers that declare the UE 5.x APIs this project uses,
  3. compiles every .cpp on its own (catches missing includes) and all of them as one unity
     translation unit (catches the name collisions UE's unity builds would hit),
     with -Wshadow as an error (UE5 treats variable shadowing as an error).

It cannot prove the code compiles against the real engine - API signatures in the stubs are
hand-written - but it catches typos, header/source mismatches and C++ errors before handing the
project to the engine.
"""
import os
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
MODULE = os.path.join(ROOT, "Source", "HadiViceCity")
STUBS = os.path.join(os.path.dirname(os.path.abspath(__file__)), "stubs")

# Engine include paths -> stub header that declares them.
ENGINE_HEADERS = {
    "UEStubEngine.h": [
        "Engine/World.h", "Engine/Engine.h", "Engine/GameInstance.h", "Engine/StaticMesh.h", "Engine/Font.h", "Engine/Canvas.h",
        "Engine/CanvasRenderTarget2D.h", "Engine/CollisionProfile.h", "Engine/LocalPlayer.h", "EngineUtils.h", "CanvasItem.h", "RenderUtils.h",
        "GameFramework/Actor.h", "GameFramework/Pawn.h", "GameFramework/Character.h", "GameFramework/Controller.h", "GameFramework/PlayerController.h",
        "GameFramework/GameModeBase.h", "GameFramework/HUD.h", "GameFramework/SaveGame.h", "GameFramework/GameUserSettings.h",
        "GameFramework/CharacterMovementComponent.h", "GameFramework/SpringArmComponent.h", "Camera/CameraComponent.h", "Camera/CameraActor.h",
        "Camera/PlayerCameraManager.h", "Components/SceneComponent.h", "Components/StaticMeshComponent.h", "Components/InstancedStaticMeshComponent.h",
        "Components/BoxComponent.h", "Components/CapsuleComponent.h", "Components/PointLightComponent.h", "Components/SpotLightComponent.h",
        "Components/DirectionalLightComponent.h", "Components/SkyLightComponent.h", "Components/SkyAtmosphereComponent.h",
        "Components/VolumetricCloudComponent.h", "Components/ExponentialHeightFogComponent.h", "Components/PostProcessComponent.h",
        "Materials/Material.h", "Materials/MaterialInstanceDynamic.h", "Kismet/GameplayStatics.h", "Kismet/KismetSystemLibrary.h",
        "Subsystems/WorldSubsystem.h", "UObject/Interface.h", "UObject/Object.h", "HAL/IConsoleManager.h", "HAL/PlatformTime.h", "Misc/App.h",
        "Misc/PackageName.h", "Modules/ModuleManager.h", "TimerManager.h", "InputCoreTypes.h",
    ],
    "UEStubInput.h": [
        "EnhancedInputComponent.h", "EnhancedInputSubsystems.h", "InputAction.h", "InputActionValue.h", "InputMappingContext.h", "InputModifiers.h",
    ],
}

CLASS_DECL = re.compile(r"\b(class|struct)\s+(?:HADIVICECITY_API\s+)?(\w+)\s*(?:final\s*)?(:\s*public\s+(\w+)[^{]*)?\{", re.S)


def transform_header(text):
    """Replace GENERATED_BODY() with what the UHT-generated macros provide."""
    text = re.sub(r'#include\s+"[^"]+\.generated\.h"\s*\n', "", text)
    out = []
    pos = 0
    for match in re.finditer(r"GENERATED_BODY\(\)", text):
        declarations = list(CLASS_DECL.finditer(text, 0, match.start()))
        replacement = ""
        if declarations:
            decl = declarations[-1]
            kind, name, base = decl.group(1), decl.group(2), decl.group(4)
            if base:
                replacement = "public: using Super = {base}; using ThisClass = {name}; static UClass* StaticClass(); private:".format(base=base, name=name)
                if kind == "struct":
                    replacement = "public: using Super = {base}; static UScriptStruct* StaticStruct();".format(base=base)
            elif kind == "struct":
                replacement = "public: static UScriptStruct* StaticStruct();"
        out.append(text[pos:match.start()])
        out.append(replacement)
        pos = match.end()
    out.append(text[pos:])
    return "".join(out)


def prepare(temp):
    module = os.path.join(temp, "HadiViceCity")
    shutil.copytree(MODULE, module)
    for directory, _, files in os.walk(module):
        for name in files:
            if name.endswith(".h") or name.endswith(".cpp"):
                path = os.path.join(directory, name)
                with open(path, encoding="utf-8") as handle:
                    text = handle.read()
                with open(path, "w", encoding="utf-8") as handle:
                    handle.write(transform_header(text))
    stub_dir = os.path.join(temp, "stubs")
    shutil.copytree(STUBS, stub_dir)
    for target, paths in ENGINE_HEADERS.items():
        for header in paths:
            path = os.path.join(stub_dir, header)
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, "w", encoding="utf-8") as handle:
                handle.write('#pragma once\n#include "{}"\n'.format(target))
    # Definitions for stub statics so the unity TU links conceptually (syntax-only anyway).
    return module, stub_dir


def compile_sources(module, stub_dir, sources, label, extra):
    cxx = os.environ.get("CXX", "clang++")
    flags = [cxx, "-std=c++20", "-fsyntax-only", "-Wall", "-Wextra", "-Wshadow", "-Werror=shadow", "-Wno-unused-parameter", "-Wno-unused-private-field",
             "-Wno-unused-variable", "-Wno-missing-field-initializers", "-Wno-invalid-offsetof", "-Wno-unused-function",
             "-I", stub_dir, "-I", os.path.join(module, "Public")] + os.environ.get("HADI_EXTRA_FLAGS", "").split() + extra
    result = subprocess.run(flags + sources, capture_output=True, text=True)
    output = (result.stdout + result.stderr).strip()
    status = "OK" if result.returncode == 0 else "FAILED"
    print("[{}] {}".format(status, label))
    if output:
        print(output)
    return result.returncode == 0


def main():
    temp = tempfile.mkdtemp(prefix="hadi_stubcheck_")
    try:
        module, stub_dir = prepare(temp)
        sources = []
        for directory, _, files in os.walk(os.path.join(module, "Private")):
            for name in sorted(files):
                if name.endswith(".cpp"):
                    sources.append(os.path.join(directory, name))
        sources.sort()
        ok = True
        for source in sources:
            ok &= compile_sources(module, stub_dir, [source], os.path.relpath(source, module), ["-include", "CoreMinimal.h"])
        unity = os.path.join(temp, "Module.HadiViceCity.cpp")
        with open(unity, "w", encoding="utf-8") as handle:
            handle.write('#include "CoreMinimal.h"\n')
            for source in sources:
                handle.write('#include "{}"\n'.format(source))
        ok &= compile_sources(module, stub_dir, [unity], "unity build (all .cpp in one TU)", [])
        print("\nStub compile check: {}".format("PASSED" if ok else "FAILED"))
        return 0 if ok else 1
    finally:
        shutil.rmtree(temp, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
