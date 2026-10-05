using UnrealBuildTool;

public class TerrainGenerator: ModuleRules
{
    public TerrainGenerator(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});

        PrivateDependencyModuleNames.AddRange(new string[] { "ProceduralMeshComponent" });
    }
}
