#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "PerformanceMetrics.generated.h"

/**
 * Performance metrics for asset analysis
 * Tracks various performance indicators during audit operations
 */
USTRUCT(BlueprintType)
struct MAGICOPTIMIZER_API FPerformanceMetrics
{
    GENERATED_BODY()

    FPerformanceMetrics()
    {
        // Initialize with default values
        MemoryUsageMB = 0.0f;
        MemoryPeakMB = 0.0f;
        ProcessingTimeMs = 0.0f;
        CPUTimeMs = 0.0f;
        GPUTimeMs = 0.0f;
        IOTimeMs = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        Vertices = 0;
        TextureMemoryMB = 0.0f;
        MeshMemoryMB = 0.0f;
        MaterialMemoryMB = 0.0f;
        BlueprintMemoryMB = 0.0f;
        CompressionRatio = 1.0f;
        QualityScore = 1.0f;
        PerformanceScore = 1.0f;
        OptimizationPotential = 0.0f;
    }

    /** Current memory usage in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory", meta = (DisplayName = "Memory Usage (MB)"))
    float MemoryUsageMB;

    /** Peak memory usage in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory", meta = (DisplayName = "Peak Memory (MB)"))
    float MemoryPeakMB;

    /** Total processing time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (DisplayName = "Processing Time (ms)"))
    float ProcessingTimeMs;

    /** CPU processing time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (DisplayName = "CPU Time (ms)"))
    float CPUTimeMs;

    /** GPU processing time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (DisplayName = "GPU Time (ms)"))
    float GPUTimeMs;

    /** I/O processing time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (DisplayName = "I/O Time (ms)"))
    float IOTimeMs;

    /** Number of draw calls */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (DisplayName = "Draw Calls"))
    int32 DrawCalls;

    /** Number of triangles */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (DisplayName = "Triangles"))
    int32 Triangles;

    /** Number of vertices */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (DisplayName = "Vertices"))
    int32 Vertices;

    /** Texture memory usage in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Breakdown", meta = (DisplayName = "Texture Memory (MB)"))
    float TextureMemoryMB;

    /** Mesh memory usage in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Breakdown", meta = (DisplayName = "Mesh Memory (MB)"))
    float MeshMemoryMB;

    /** Material memory usage in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Breakdown", meta = (DisplayName = "Material Memory (MB)"))
    float MaterialMemoryMB;

    /** Blueprint memory usage in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Breakdown", meta = (DisplayName = "Blueprint Memory (MB)"))
    float BlueprintMemoryMB;

    /** Compression ratio (1.0 = no compression, < 1.0 = compressed) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (DisplayName = "Compression Ratio"))
    float CompressionRatio;

    /** Quality score (0.0 = poor, 1.0 = excellent) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality", meta = (DisplayName = "Quality Score"))
    float QualityScore;

    /** Performance score (0.0 = poor, 1.0 = excellent) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (DisplayName = "Performance Score"))
    float PerformanceScore;

    /** Optimization potential (0.0 = no optimization, 1.0 = maximum optimization) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (DisplayName = "Optimization Potential"))
    float OptimizationPotential;

    /** Additional custom metrics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom", meta = (DisplayName = "Custom Metrics"))
    TMap<FString, float> CustomMetrics;

    /**
     * Calculate total memory usage
     * @return Total memory usage in MB
     */
    float GetTotalMemoryUsage() const
    {
        return TextureMemoryMB + MeshMemoryMB + MaterialMemoryMB + BlueprintMemoryMB;
    }

    /**
     * Calculate memory efficiency score
     * @return Memory efficiency score (0.0 = poor, 1.0 = excellent)
     */
    float GetMemoryEfficiencyScore() const
    {
        if (MemoryPeakMB <= 0.0f)
        {
            return 1.0f;
        }
        
        float Efficiency = MemoryUsageMB / MemoryPeakMB;
        return FMath::Clamp(Efficiency, 0.0f, 1.0f);
    }

    /**
     * Calculate processing efficiency score
     * @return Processing efficiency score (0.0 = poor, 1.0 = excellent)
     */
    float GetProcessingEfficiencyScore() const
    {
        if (ProcessingTimeMs <= 0.0f)
        {
            return 1.0f;
        }
        
        // Lower processing time = higher efficiency
        float Efficiency = 1000.0f / ProcessingTimeMs; // Normalize to 1 second
        return FMath::Clamp(Efficiency, 0.0f, 1.0f);
    }

    /**
     * Calculate overall performance score
     * @return Overall performance score (0.0 = poor, 1.0 = excellent)
     */
    float GetOverallPerformanceScore() const
    {
        float MemoryScore = GetMemoryEfficiencyScore();
        float ProcessingScore = GetProcessingEfficiencyScore();
        float QualityScoreValue = this->QualityScore;
        float PerformanceScoreValue = this->PerformanceScore;
        
        // Weighted average
        return (MemoryScore * 0.3f + ProcessingScore * 0.3f + QualityScoreValue * 0.2f + PerformanceScoreValue * 0.2f);
    }

    /**
     * Check if performance meets target thresholds
     * @param MaxMemoryMB Maximum memory usage in MB
     * @param MaxDrawCalls Maximum draw calls
     * @param MaxProcessingTimeMs Maximum processing time in ms
     * @return True if performance meets targets
     */
    bool MeetsPerformanceTargets(float MaxMemoryMB, int32 MaxDrawCalls, float MaxProcessingTimeMs) const
    {
        return (MemoryUsageMB <= MaxMemoryMB) && 
               (DrawCalls <= MaxDrawCalls) && 
               (ProcessingTimeMs <= MaxProcessingTimeMs);
    }

    /**
     * Get performance summary as string
     * @return Performance summary string
     */
    FString GetPerformanceSummary() const
    {
        return FString::Printf(TEXT("Memory: %.2fMB, DrawCalls: %d, Time: %.2fms, Quality: %.2f, Performance: %.2f"),
            MemoryUsageMB, DrawCalls, ProcessingTimeMs, QualityScore, PerformanceScore);
    }

    /**
     * Add custom metric
     * @param Name Metric name
     * @param Value Metric value
     */
    void AddCustomMetric(const FString& Name, float Value)
    {
        CustomMetrics.Add(Name, Value);
    }

    /**
     * Get custom metric value
     * @param Name Metric name
     * @param DefaultValue Default value if metric not found
     * @return Metric value
     */
    float GetCustomMetric(const FString& Name, float DefaultValue = 0.0f) const
    {
        if (const float* Value = CustomMetrics.Find(Name))
        {
            return *Value;
        }
        return DefaultValue;
    }

    /**
     * Reset all metrics to default values
     */
    void Reset()
    {
        MemoryUsageMB = 0.0f;
        MemoryPeakMB = 0.0f;
        ProcessingTimeMs = 0.0f;
        CPUTimeMs = 0.0f;
        GPUTimeMs = 0.0f;
        IOTimeMs = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        Vertices = 0;
        TextureMemoryMB = 0.0f;
        MeshMemoryMB = 0.0f;
        MaterialMemoryMB = 0.0f;
        BlueprintMemoryMB = 0.0f;
        CompressionRatio = 1.0f;
        QualityScore = 1.0f;
        PerformanceScore = 1.0f;
        OptimizationPotential = 0.0f;
        CustomMetrics.Empty();
    }

    /**
     * Combine with another performance metrics
     * @param Other Other performance metrics to combine with
     */
    void CombineWith(const FPerformanceMetrics& Other)
    {
        MemoryUsageMB += Other.MemoryUsageMB;
        MemoryPeakMB = FMath::Max(MemoryPeakMB, Other.MemoryPeakMB);
        ProcessingTimeMs += Other.ProcessingTimeMs;
        CPUTimeMs += Other.CPUTimeMs;
        GPUTimeMs += Other.GPUTimeMs;
        IOTimeMs += Other.IOTimeMs;
        DrawCalls += Other.DrawCalls;
        Triangles += Other.Triangles;
        Vertices += Other.Vertices;
        TextureMemoryMB += Other.TextureMemoryMB;
        MeshMemoryMB += Other.MeshMemoryMB;
        MaterialMemoryMB += Other.MaterialMemoryMB;
        BlueprintMemoryMB += Other.BlueprintMemoryMB;
        
        // Average quality and performance scores
        this->QualityScore = (this->QualityScore + Other.QualityScore) * 0.5f;
        this->PerformanceScore = (this->PerformanceScore + Other.PerformanceScore) * 0.5f;
        OptimizationPotential = (OptimizationPotential + Other.OptimizationPotential) * 0.5f;
        
        // Combine custom metrics
        for (const auto& Pair : Other.CustomMetrics)
        {
            if (float* ExistingValue = CustomMetrics.Find(Pair.Key))
            {
                *ExistingValue += Pair.Value;
            }
            else
            {
                CustomMetrics.Add(Pair.Key, Pair.Value);
            }
        }
    }
};
