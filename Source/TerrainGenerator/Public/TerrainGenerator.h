#pragma once

#include "GameFramework/Actor.h"
#include "TerrainDecoration.h"
#include "TerrainHeightField.h"
#include "TerrainTypes.h"
#include "TerrainGenerator.generated.h"

class UBoxComponent;
class UHierarchicalInstancedStaticMeshComponent;
class UProceduralMeshComponent;
class UStaticMeshComponent;
class UMaterialInterface;

/**
 * Generates a noise terrain centered at the actor origin (place the actor at 0,0,0), optionally with a water plane.
 * Island: interior heights from fBm Perlin noise; near the borders the ground smoothly sinks below the water.
 * Continent: land to the edges, no sea. WaterOnly: just water.
 * Corridors shape a dry, gentle strip (e.g. a gameplay lane); a decoration set scatters instanced props beside it.
 * Meant to be generated in the editor and saved: nothing regenerates at BeginPlay unless the saved data is missing.
 */
UCLASS()
class TERRAINGENERATOR_API ATerrainGenerator : public AActor
{
	GENERATED_BODY()

public:
	ATerrainGenerator();

	/** What to build. Changing it in the editor applies that type's defaults (MinLandHeight, bWater); both stay editable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Type")
	ETerrainType TerrainType = ETerrainType::Island;

	/** Full ground size along X, cm (200000 = 2 km). Ground spans [-SizeX/2, SizeX/2]. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Ground Size", meta = (ClampMin = "1000"))
	float SizeX = 200000.f;

	/** Full ground size along Y, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Ground Size", meta = (ClampMin = "1000"))
	float SizeY = 200000.f;

	/** Highest possible hill top above the water level, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Ground Size", meta = (ClampMin = "0"))
	float MaxHeight = 4000.f;

	/** Lowest inland ground relative to the water level, cm. Negative = shallow ponds (Island) or dry valleys (Continent). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Ground Size")
	float MinLandHeight = 200.f;

	/** Distance between grid vertices, cm, used where CellSizeX / CellSizeY are 0. Larger = fewer triangles (important on mobile). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Ground Size", meta = (ClampMin = "100"))
	float CellSize = 10000.f;

	/** Grid spacing along X, cm. 0 = CellSize. A side view wants fine X and coarse Y. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Ground Size", meta = (ClampMin = "0"))
	float CellSizeX = 0.f;

	/** Grid spacing along Y, cm. 0 = CellSize. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Ground Size", meta = (ClampMin = "0"))
	float CellSizeY = 0.f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Shape")
	int32 Seed = 1337;

	/** Pick a new random seed every time the game starts. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Shape")
	bool bRandomSeedOnBeginPlay = false;

	/** Base noise frequency per cm. Smaller = wider, smoother hills. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Shape", meta = (ClampMin = "0.000001"))
	float NoiseScale = 0.00004f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Shape", meta = (ClampMin = "1", ClampMax = "8"))
	int32 Octaves = 4;

	/** Amplitude multiplier per octave (lower = smoother). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Shape", meta = (ClampMin = "0", ClampMax = "1"))
	float Persistence = 0.45f;

	/** Frequency multiplier per octave. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Shape", meta = (ClampMin = "1"))
	float Lacunarity = 2.f;

	/** >1 flattens lowlands and sharpens peaks (more flat area for ground enemies). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Shape", meta = (ClampMin = "0.1"))
	float Flatness = 1.6f;

	/** Share of ridged noise on the high ground: ridges and peaks instead of round hills (0 = off). Corridor profiles ignore it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Shape", meta = (ClampMin = "0", ClampMax = "1"))
	float RidgeWeight = 0.f;

	/** Normalised noise height (0..1) where the ridges start to blend in. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Shape", meta = (ClampMin = "0", ClampMax = "1"))
	float RidgeStart = 0.35f;

	/** Shaped strips (e.g. the gameplay lane), applied in order after the island mask. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Shape")
	TArray<FTerrainCorridor> Corridors;


	/** Fraction of the half-size over which the ground slopes down into the water (Island only). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Shore", meta = (ClampMin = "0.01", ClampMax = "1"))
	float ShoreWidth = 0.2f;

	/** How far below the water level the ground goes at the very border, cm (Island only). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Shore", meta = (ClampMin = "0"))
	float SeaDepth = 1000.f;

	/** How irregular the coastline is (0 = rectangular shore, Island only). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Shore", meta = (ClampMin = "0", ClampMax = "1"))
	float CoastlineNoise = 0.6f;


	/** Builds the water plane and its solid floor. The water surface blocks like the ground does. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Water")
	bool bWater = true;

	/** Water surface Z relative to the actor, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Water")
	float WaterLevel = 0.f;

	/** Water plane size as a multiple of the largest ground side (big enough to reach the horizon). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Water", meta = (ClampMin = "1"))
	float WaterExtent = 6.f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Rendering")
	TObjectPtr<UMaterialInterface> GroundMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Rendering")
	TObjectPtr<UMaterialInterface> WaterMaterial;

	/** World size of one ground texture tile, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Rendering", meta = (ClampMin = "1"))
	float UVTileSize = 2000.f;

	/**
	 * Baked occlusion into vertex colour B (0 open .. 1 dark), for the ground material to darken valleys and slope bottoms:
	 * each vertex is compared with the mean ground on rings at this radius and half of it, cm. 0 = off.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Rendering", meta = (ClampMin = "0"))
	float OcclusionRadius = 3000.f;

	/** How far below that mean the ground must lie for full occlusion, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Rendering", meta = (ClampMin = "1"))
	float OcclusionDepth = 600.f;

	/** Strength of the soft dark patch decorations leave on the ground (vertex colour B), 0..1. 0 = off. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Rendering", meta = (ClampMin = "0", ClampMax = "1"))
	float ContactDarkening = 0.6f;

	/** Contact patch radius as a multiple of the decoration's scaled horizontal reach. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Rendering", meta = (ClampMin = "0"))
	float ContactRadiusScale = 1.3f;

	/**
	 * Index into Corridors whose centre line is painted as a path (a track or road) into vertex colour A, as 1 - mask:
	 * A = 1 is no path. -1 = off.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Rendering", meta = (ClampMin = "-1"))
	int32 PathCorridorIndex = -1;

	/** Half-width of the painted path, cm: the mask falls linearly from 1 on the centre line to 0 here. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Rendering", meta = (ClampMin = "1"))
	float PathHalfWidth = 600.f;


	/** Decorations to scatter beside the decoration corridor. None = no decorations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Decorations")
	TObjectPtr<UTerrainDecorationSet> DecorationSet;

	/** Index into Corridors that decorations line up along. Without that corridor they span the whole ground at Y = 0. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Decorations", meta = (ClampMin = "0"))
	int32 DecorationCorridorIndex = 0;

	/** Mixed into Seed for the decoration stream, so re-rolling decorations doesn't change the ground. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Decorations")
	int32 DecorationSeedSalt = 0x44454352; // 'DECR'


	/** Rebuild on every construction (editor edits and drags). Turn off on large terrains; the Generate button still works. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation")
	bool bGenerateOnConstruction = true;

	/** Cook collision off the game thread. Baked terrains load cooked collision, so this rarely matters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Generation")
	bool bAsyncCollisionCooking = true;


	/** Rebuilds the ground mesh, collision, water and decorations from the current parameters. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain")
	void Generate();

	/** Re-scatters decorations only (the ground must already be generated). */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain")
	void GenerateDecorations();

	/** Sets MinLandHeight and bWater for the current TerrainType. Called when the type changes in the editor. */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void ApplyTypeDefaults();

	/** Exact ground Z (world) under a world XY point; matches the rendered triangles. WaterOnly: the water Z. */
	UFUNCTION(BlueprintPure, Category = "Terrain")
	float GetHeightAt(FVector2D WorldXY) const;

	/** World Z of whatever is solid on top: the higher of the ground and (if there is water) the water surface. */
	UFUNCTION(BlueprintPure, Category = "Terrain")
	float GetSurfaceHeightAt(FVector2D WorldXY) const;

	/** True if the world XY point is dry land (above water by at least MinHeightAboveWater). Always false for WaterOnly. */
	UFUNCTION(BlueprintPure, Category = "Terrain")
	bool IsLand(FVector2D WorldXY, float MinHeightAboveWater = 100.f) const;

	/** World Z of the water surface. */
	UFUNCTION(BlueprintPure, Category = "Terrain")
	float GetWaterZ() const;

	/**
	 * Random point on dry, not-too-steep ground for spawning enemies.
	 * Returns false if nothing suitable was found within MaxAttempts.
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	bool FindSpawnPoint(FVector& OutLocation, float MinHeightAboveWater = 100.f, float MaxSlopeDegrees = 25.f, int32 MaxAttempts = 64);

	/** The parameters Generate() feeds the height field with. */
	FTerrainHeightParams MakeHeightParams() const;

	/**
	 * The grid Generate() uses for the current size and cell settings. An Island whose land-width corridor ends near the +X
	 * edge gets finer columns from that end on (FTerrainGrid fine end), where its outline curves across the grid.
	 */
	FTerrainGrid MakeGrid() const;

	/** The grid the saved heights were built on: MakeGrid(), or a regular one for heights saved before the fine end. */
	FTerrainGrid GetGrid() const;

	/** Saved heights, row-major, relative to the actor. Empty for WaterOnly. */
	const TArray<float>& GetHeights() const { return Heights; }

	/** Decoration components built by GenerateDecorations(), one per mesh. */
	const TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>>& GetDecorationComponents() const { return DecorationComponents; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void BuildGroundMesh();
	void UpdateWater();
	void ClearDecorations();

	/** Vertex colour B of the ground: baked occlusion plus a contact patch under every current decoration instance. */
	void UpdateGroundDarkness();

	/** Ground height above water and normal at a local XY point, from the saved heights. */
	float LocalHeightAboveWater(const FVector2D& LocalXY) const;
	FVector LocalNormal(const FVector2D& LocalXY) const;

	UPROPERTY(VisibleAnywhere, Category = "Terrain")
	TObjectPtr<UProceduralMeshComponent> Ground;

	/** Visual only; collision comes from WaterFloor. */
	UPROPERTY(VisibleAnywhere, Category = "Terrain")
	TObjectPtr<UStaticMeshComponent> Water;

	/** Solid water: BlockAll (WorldStatic, like Ground), top face at WaterLevel, active whenever bWater is on. */
	UPROPERTY(VisibleAnywhere, Category = "Terrain")
	TObjectPtr<UBoxComponent> WaterFloor;

	UPROPERTY()
	TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> DecorationComponents;

	// Serialized so height queries work in cooked builds without regenerating.
	UPROPERTY()
	TArray<float> Heights;

	UPROPERTY()
	int32 NumX = 0;

	UPROPERTY()
	int32 NumY = 0;

	UPROPERTY()
	float StepX = 0.f;

	UPROPERTY()
	float StepY = 0.f;

	FRandomStream SpawnStream;
};
