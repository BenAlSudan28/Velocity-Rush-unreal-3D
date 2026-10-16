#!/usr/bin/env python3
"""Static sanity checks for the source-only Unreal project (not a substitute for UBT)."""
from __future__ import annotations

import json
import math
import re
import sys
import wave
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
errors: list[str] = []
checks = 0


def require(condition: bool, message: str) -> None:
    global checks
    checks += 1
    if not condition:
        errors.append(message)


required = [
    "VelocityRush.uproject",
    "Source/VelocityRush/VelocityRush.Build.cs",
    "Source/VelocityRush.Target.cs",
    "Source/VelocityRushEditor.Target.cs",
    "Source/VelocityRush/Public/VelocityRushGameMode.h",
    "Source/VelocityRush/Private/VelocityRushGameMode.cpp",
    "Source/VelocityRush/Public/VelocityRushPlayerController.h",
    "Source/VelocityRush/Private/VelocityRushPlayerController.cpp",
    "Source/VelocityRush/Public/VelocityRushGameUI.h",
    "Source/VelocityRush/Private/VelocityRushGameUI.cpp",
    "Source/VelocityRush/Public/VelocityRushTrackActor.h",
    "Source/VelocityRush/Private/VelocityRushTrackActor.cpp",
    "Source/VelocityRush/Public/VelocityRushVehiclePawn.h",
    "Source/VelocityRush/Private/VelocityRushVehiclePawn.cpp",
    "Source/VelocityRush/Public/VelocityRushPickup.h",
    "Source/VelocityRush/Private/VelocityRushPickup.cpp",
    "Config/DefaultEngine.ini",
    "Config/DefaultInput.ini",
    ".github/workflows/build-unreal-android.yml",
    "Tools/BuildAndroid.ps1",
]
for item in required:
    require((ROOT / item).is_file(), f"Missing required file: {item}")

try:
    project = json.loads((ROOT / "VelocityRush.uproject").read_text(encoding="utf-8"))
    require(project.get("EngineAssociation") == "5.5", "Unreal Engine association must be 5.5")
    modules = {module.get("Name") for module in project.get("Modules", [])}
    require("VelocityRush" in modules, "Runtime module VelocityRush missing from uproject")
    plugins = {plugin.get("Name") for plugin in project.get("Plugins", []) if plugin.get("Enabled")}
    require("ProceduralMeshComponent" in plugins, "ProceduralMeshComponent plugin is not enabled")
except Exception as exc:
    errors.append(f"Cannot parse VelocityRush.uproject: {exc}")

engine_ini = (ROOT / "Config/DefaultEngine.ini").read_text(encoding="utf-8")
input_ini = (ROOT / "Config/DefaultInput.ini").read_text(encoding="utf-8")
require("/Script/VelocityRush.VelocityRushGameMode" in engine_ini, "Default GameMode is not registered in DefaultEngine.ini")
require("PackageName=com.velocityrush.studio" in engine_ini, "Android package name is missing")
require("Orientation=Landscape" in engine_ini, "Android landscape orientation is missing")
require('MapsToCook=(FilePath="/Engine/Maps/Entry")' in engine_ini, "Runtime-generated Entry map is not explicitly included for cooking")
require('AxisName="Throttle"' in input_ini and 'AxisName="Steering"' in input_ini, "Keyboard throttle/steering bindings are incomplete")
require('AxisName="Brake"' in input_ini and 'ActionName="Nitro"' in input_ini, "Brake/nitro input bindings are incomplete")

# Check quoted local includes, generated headers, and basic brace balance.
for source in sorted((ROOT / "Source").rglob("*")):
    if source.suffix not in {".h", ".cpp"}:
        continue
    text = source.read_text(encoding="utf-8")
    if source.suffix == ".h" and "UCLASS(" in text:
        generated = re.findall(r'^#include\s+"([^"]+\.generated\.h)"\s*$', text, re.M)
        require(len(generated) == 1, f"Expected one generated header in {source.relative_to(ROOT)}")
        include_lines = re.findall(r'^#include\s+"[^"]+"\s*$', text, re.M)
        require(bool(include_lines) and generated[0] in include_lines[-1], f"Generated header must be the last include in {source.relative_to(ROOT)}")
    for header in re.findall(r'^#include\s+"([^"]+)"', text, re.M):
        if header.endswith(".generated.h"):
            continue
        if not header.startswith("VelocityRush"):
            continue
        candidates = [ROOT / "Source/VelocityRush/Public" / header, ROOT / "Source/VelocityRush/Private" / header]
        require(any(candidate.is_file() for candidate in candidates), f"Unresolved project-local include {header} in {source.relative_to(ROOT)}")
    # The current source avoids brace characters inside string literals; this catches common edit truncations.
    require(text.count("{") == text.count("}"), f"Unbalanced braces in {source.relative_to(ROOT)}")

# Feature wiring: not merely a checklist of names; require both declarations and implementations.
features = {
    "curved route generation": ("Source/VelocityRush/Private/VelocityRushTrackActor.cpp", "BuildRoute()"),
    "road mesh generation": ("Source/VelocityRush/Private/VelocityRushTrackActor.cpp", "CreateMeshSection_LinearColor"),
    "procedural city": ("Source/VelocityRush/Private/VelocityRushTrackActor.cpp", "BuildCity()"),
    "rival spawning": ("Source/VelocityRush/Private/VelocityRushGameMode.cpp", "SpawnRivals()"),
    "fuel/nitro pickup spawning": ("Source/VelocityRush/Private/VelocityRushGameMode.cpp", "SpawnPickups()"),
    "fuel refill": ("Source/VelocityRush/Private/VelocityRushPickup.cpp", "AddFuel(32.0f)"),
    "nitro refill": ("Source/VelocityRush/Private/VelocityRushPickup.cpp", "AddNitro(45.0f)"),
    "touch controls": ("Source/VelocityRush/Private/VelocityRushGameUI.cpp", "HandleThrottlePressed"),
    "fuel HUD": ("Source/VelocityRush/Private/VelocityRushGameUI.cpp", "FuelBar->SetPercent"),
    "nitro HUD": ("Source/VelocityRush/Private/VelocityRushGameUI.cpp", "NitroBar->SetPercent"),
    "pause/resume": ("Source/VelocityRush/Private/VelocityRushGameMode.cpp", "SetRacePaused"),
    "optional imported vehicle models": ("Source/VelocityRush/Private/VelocityRushVehiclePawn.cpp", "/Game/Vehicles/AerionGT.AerionGT"),
    "pause action in PlayerController": ("Source/VelocityRush/Private/VelocityRushPlayerController.cpp", 'BindAction(TEXT("Pause")'),
    "multi-touch state reset": ("Source/VelocityRush/Private/VelocityRushGameUI.cpp", "ClearTouchState"),
    "persistent collision penalty": ("Source/VelocityRush/Private/VelocityRushVehiclePawn.cpp", "- CollisionPenalty"),
}
for label, (relative, needle) in features.items():
    require(needle in (ROOT / relative).read_text(encoding="utf-8"), f"Missing feature wiring: {label}")

# Rebuild the authored Catmull-Rom centerline numerically to catch a straight or degenerate track.
track_source = (ROOT / "Source/VelocityRush/Private/VelocityRushTrackActor.cpp").read_text(encoding="utf-8")
control_block = re.search(r"const TArray<FVector> ControlPoints\s*=\s*\{(.*?)\};", track_source, re.S)
sample_match = re.search(r"constexpr int32 SamplesPerSegment\s*=\s*(\d+)", track_source)
if control_block and sample_match:
    control = [tuple(map(float, values)) for values in re.findall(
        r"FVector\((-?[\d.]+)f,\s*(-?[\d.]+)f,\s*(-?[\d.]+)f\)", control_block.group(1))]
    samples_per_segment = int(sample_match.group(1))
    route_points: list[tuple[float, float, float]] = []
    for index in range(len(control)):
        p0, p1, p2, p3 = [control[(index + offset) % len(control)] for offset in (-1, 0, 1, 2)]
        for sample in range(samples_per_segment):
            t = sample / samples_per_segment
            t2, t3 = t * t, t * t * t
            route_points.append(tuple(0.5 * (2 * p1[axis] + (-p0[axis] + p2[axis]) * t +
                (2 * p0[axis] - 5 * p1[axis] + 4 * p2[axis] - p3[axis]) * t2 +
                (-p0[axis] + 3 * p1[axis] - 3 * p2[axis] + p3[axis]) * t3) for axis in range(3)))
    route_length = sum(math.dist(route_points[index], route_points[(index + 1) % len(route_points)])
                       for index in range(len(route_points))) if route_points else 0.0
    turn_samples = 0
    for index, point in enumerate(route_points):
        previous, following = route_points[index - 1], route_points[(index + 1) % len(route_points)]
        before = (point[0] - previous[0], point[1] - previous[1])
        after = (following[0] - point[0], following[1] - point[1])
        denominator = math.hypot(*before) * math.hypot(*after)
        if denominator > 0:
            cosine = max(-1.0, min(1.0, (before[0] * after[0] + before[1] * after[1]) / denominator))
            if math.degrees(math.acos(cosine)) > 2.0:
                turn_samples += 1
    require(len(control) >= 6 and len(route_points) >= 100, "Circuit centerline is missing or undersampled")
    require(route_length >= 20000.0, f"Circuit is too short: {route_length:.0f} cm")
    require(turn_samples >= 40, f"Circuit lacks turns: {turn_samples} turning samples")
else:
    errors.append("Cannot find control points/samples in the procedural track source")

workflow_path = ROOT / ".github/workflows/build-unreal-android.yml"
workflow_text = workflow_path.read_text(encoding="utf-8")
require("actions/checkout@v4" in workflow_text and "actions/upload-artifact@v4" in workflow_text, "Workflow actions are missing or not pinned to major versions")
require("self-hosted" in workflow_text and "UE_ROOT" in workflow_text, "Workflow must target a configured Unreal self-hosted runner")
require("Content/**" in workflow_text, "Workflow must run when Unreal Content assets change")
require("-platform=Android" in workflow_text and "-cookflavor=ETC2" in workflow_text, "Workflow Android UAT arguments are incomplete")
require("no APK was created" in workflow_text and "if-no-files-found: error" in workflow_text, "Workflow must fail when no APK artifact is produced")
try:
    import yaml  # type: ignore
    parsed = yaml.load(workflow_text, Loader=yaml.BaseLoader)
    require(isinstance(parsed, dict) and "jobs" in parsed and "android-apk" in parsed.get("jobs", {}), "Workflow YAML lacks the expected Android job")
except ImportError:
    print("NOTE: PyYAML is unavailable; Workflow YAML syntax was not parser-validated.")
except Exception as exc:
    errors.append(f"Workflow YAML parse failed: {exc}")

ui_header = (ROOT / "Source/VelocityRush/Public/VelocityRushGameUI.h").read_text(encoding="utf-8")
ui_source = (ROOT / "Source/VelocityRush/Private/VelocityRushGameUI.cpp").read_text(encoding="utf-8")
ui_handlers = re.findall(r"UFUNCTION\(\)\s*void\s+(Handle\w+)\s*\(", ui_header)
for handler in ui_handlers:
    require(f"URushGameUI::{handler}(" in ui_source, f"UFUNCTION handler has no implementation: {handler}")
for handler in ("HandleLeftReleased", "HandleRightReleased", "HandleThrottleReleased", "HandleBrakeReleased", "HandleNitroReleased"):
    require(f"&URushGameUI::{handler}" in ui_source, f"Touch release is not wired: {handler}")

# Check WAV containers and the included Arabic-font license.
wav_files = sorted((ROOT / "Resources/Audio").rglob("*.wav"))
require(len(wav_files) >= 1, "No source WAV files found")
for wav_path in wav_files:
    try:
        with wave.open(str(wav_path), "rb") as stream:
            require(stream.getnchannels() > 0 and stream.getframerate() > 0 and stream.getnframes() > 0,
                    f"Invalid/empty WAV: {wav_path.relative_to(ROOT)}")
    except Exception as exc:
        errors.append(f"Unreadable WAV {wav_path.relative_to(ROOT)}: {exc}")
require((ROOT / "Resources/Fonts/OFL.txt").is_file(), "Noto Arabic font license file missing")
require((ROOT / "Resources/Fonts/NotoSansArabic-Regular.ttf").is_file(), "Arabic regular font source missing")
for model_path in sorted((ROOT / "Resources/Models").glob("*.glb")):
    try:
        with model_path.open("rb") as model_file:
            require(model_file.read(4) == b"glTF", f"Invalid GLB header: {model_path.name}")
        require(model_path.stat().st_size > 1024, f"GLB is suspiciously small: {model_path.name}")
    except OSError as exc:
        errors.append(f"Cannot read GLB {model_path.name}: {exc}")

# The source package deliberately excludes the retired Godot project.
for pattern in ("*.gd", "*.tscn", "project.godot"):
    require(not any(ROOT.rglob(pattern)), f"Godot file remains in the Unreal package: {pattern}")

if errors:
    print(f"FAIL: {len(errors)} issue(s) across {checks} checks")
    for error in errors:
        print(f" - {error}")
    sys.exit(1)
print(f"PASS: {checks} static checks; {len(wav_files)} valid WAV source files.")
print("LIMIT: this does not compile C++ with UnrealBuildTool, cook assets, or build an APK.")
