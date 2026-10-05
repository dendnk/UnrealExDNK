#pragma once

#include "GameFramework/Actor.h"
#include "TerrainGenerator.generated.h"

class UProceduralMeshComponent;
class UStaticMeshComponent;
class UMaterialInterface;

/**
 * Generates an island-style terrain centered at the actor origin (place the actor at 0,0,0),
 * surrounded by a flat water plane. Interior heights come from fBm Perlin noise; near the
 * borders the ground smoothly sinks below the water, so the playable land is always enclosed.
 */
UCLASS()
class ATerrainGenerator : public AActor
{
	GENERATED_BODY()

public:
	ATerrainGenerator();

	/** Full ground size along X, cm (200000 = 2 km). Ground spans [-SizeX/2, SizeX/2]. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground|Ground Size", meta = (ClampMin = "1000"))
	float SizeX = 200000.f;

	/** Full ground size along Y, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Ground Size", meta = (ClampMin = "1000"))
	float SizeY = 200000.f;

	/** Highest possible hill top above the water level, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Ground Size", meta = (ClampMin = "0"))
	float MaxHeight = 4000.f;

	/** Lowest land height above the water level in the interior, cm (keeps valleys dry). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Ground Size", meta = (ClampMin = "0"))
	float MinLandHeight = 200.f;

	/** Distance between grid vertices, cm. Larger = fewer triangles (important on mobile). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Ground Size", meta = (ClampMin = "100"))
	float CellSize = 10000.f;


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


	/** Fraction of the half-size over which the ground slopes down into the water. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Shore", meta = (ClampMin = "0.01", ClampMax = "1"))
	float ShoreWidth = 0.2f;

	/** How far below the water level the ground goes at the very border, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Shore", meta = (ClampMin = "0"))
	float SeaDepth = 1000.f;

	/** How irregular the coastline is (0 = rectangular shore). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Shore", meta = (ClampMin = "0", ClampMax = "1"))
	float CoastlineNoise = 0.6f;


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


	/** Rebuilds the ground mesh, collision and water plane from the current parameters. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain")
	void Generate();

	/** Exact ground Z (world) under a world XY point; matches the rendered triangles. */
	UFUNCTION(BlueprintPure, Category = "Terrain")
	float GetHeightAt(FVector2D WorldXY) const;

	/** True if the world XY point is dry land (above water by at least MinHeightAboveWater). */
	UFUNCTION(BlueprintPure, Category = "Terrain")
	bool IsLand(FVector2D WorldXY, float MinHeightAboveWater = 100.f) const;

	/**
	 * Random point on dry, not-too-steep ground for spawning enemies.
	 * Returns false if nothing suitable was found within MaxAttempts.
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	bool FindSpawnPoint(FVector& OutLocation, float MinHeightAboveWater = 100.f, float MaxSlopeDegrees = 25.f, int32 MaxAttempts = 64);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

private:
	/** Terrain height (relative to actor) at a local XY position. */
	float SampleHeight(float X, float Y) const;

	UPROPERTY(VisibleAnywhere, Category = "Terrain")
	TObjectPtr<UProceduralMeshComponent> Ground;

	UPROPERTY(VisibleAnywhere, Category = "Terrain")
	TObjectPtr<UStaticMeshComponent> Water;

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

	FVector2D NoiseOffset = FVector2D::ZeroVector;
	FRandomStream SpawnStream;
};
