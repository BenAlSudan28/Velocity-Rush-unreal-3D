using UnrealBuildTool;
using System.Collections.Generic;

public class VelocityRushEditorTarget : TargetRules
{
    public VelocityRushEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        ExtraModuleNames.Add("VelocityRush");
    }
}
