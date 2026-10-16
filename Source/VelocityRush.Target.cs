using UnrealBuildTool;
using System.Collections.Generic;

public class VelocityRushTarget : TargetRules
{
    public VelocityRushTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        ExtraModuleNames.Add("VelocityRush");
    }
}
