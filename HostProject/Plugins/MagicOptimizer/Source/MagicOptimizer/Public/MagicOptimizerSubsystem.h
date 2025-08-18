// Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Engine/Engine.h"
#include "MagicOptimizerSubsystem.generated.h"

class UOptimizerSettings;

/**
 * Engine-level subsystem for MagicOptimizer that provides optimization services
 * throughout the engine's lifetime. This follows UE5.6 best practices for
 * plugin architecture and lifecycle management.
 */
UCLASS(ClassGroup=Optimization, BlueprintType)
class MAGICOPTIMIZER_API UMagicOptimizerSubsystem : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    UMagicOptimizerSubsystem();

    // Engine Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Optimization API
    UFUNCTION(BlueprintCallable, Category="Optimization", meta=(DisplayName="Run Optimization Pass"))
    void RunOptimizationPass();

    UFUNCTION(BlueprintCallable, Category="Optimization", meta=(DisplayName="Run Texture Optimization"))
    void RunTextureOptimization();

    UFUNCTION(BlueprintCallable, Category="Optimization", meta=(DisplayName="Run Mesh Optimization"))
    void RunMeshOptimization();

    UFUNCTION(BlueprintCallable, Category="Optimization", meta=(DisplayName="Run Material Optimization"))
    void RunMaterialOptimization();

    // Status and control
    UFUNCTION(BlueprintPure, Category="Optimization")
    bool IsOptimizationRunning() const { return bOptimizationRunning; }

    UFUNCTION(BlueprintPure, Category="Optimization")
    float GetLastOptimizationTime() const { return LastOptimizationTime; }

    UFUNCTION(BlueprintPure, Category="Optimization")
    int32 GetOptimizedAssetCount() const { return OptimizedAssetCount; }

    // Settings access
    UFUNCTION(BlueprintPure, Category="Optimization")
    UOptimizerSettings* GetOptimizerSettings() const;

protected:
    // Internal optimization methods
    void ExecuteOptimizationPass();
    void OnOptimizationComplete(bool bSuccess, const FString& ResultMessage);

    // Performance tracking
    void StartPerformanceTracking();
    void EndPerformanceTracking();

private:
    // State tracking
    UPROPERTY()
    bool bOptimizationRunning;

    UPROPERTY()
    float LastOptimizationTime;

    UPROPERTY()
    int32 OptimizedAssetCount;

    // Performance tracking
    double OptimizationStartTime;
    FString CurrentOptimizationType;
};
