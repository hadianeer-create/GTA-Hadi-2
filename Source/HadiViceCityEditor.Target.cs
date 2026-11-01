// HADI: VICE CITY - editor target.
using UnrealBuildTool;
using System.Collections.Generic;

public class HadiViceCityEditorTarget : TargetRules
{
	public HadiViceCityEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("HadiViceCity");
	}
}
