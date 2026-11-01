// HADI: VICE CITY - game target.
using UnrealBuildTool;
using System.Collections.Generic;

public class HadiViceCityTarget : TargetRules
{
	public HadiViceCityTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("HadiViceCity");
	}
}
