#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

/**
 * MagicOptimizer Diagnostics and Reporting System
 * 
 * Provides comprehensive logging, performance monitoring, and diagnostic reporting
 * for thorough plugin testing and analysis.
 */
class MAGICOPTIMIZER_API FMagicOptimizerDiagnostics
{
public:
    // Diagnostic categories
    enum class EDiagnosticCategory : uint8
    {
        General = 0,
        Performance,
        Memory,
        Settings,
        UI,
        AssetProcessing,
        Error,
        Warning,
        Info
    };

    // Performance metrics
    struct FPerformanceMetrics
    {
        double StartTime;
        double EndTime;
        double Duration;
        uint64 MemoryUsage;
        uint32 AssetCount;
        FString OperationName;
        
        FPerformanceMetrics() : StartTime(0.0), EndTime(0.0), Duration(0.0), MemoryUsage(0), AssetCount(0) {}
    };

    // Initialize diagnostics system
    static void Initialize();
    
    // Shutdown diagnostics system
    static void Shutdown();
    
    // Enhanced logging with categories
    static void LogMessage(EDiagnosticCategory Category, const FString& Message, bool bOnScreen = false, float OnScreenDuration = 5.0f);
    
    // Performance monitoring
    static void StartPerformanceTimer(const FString& OperationName);
    static void EndPerformanceTimer(const FString& OperationName);
    static FPerformanceMetrics GetPerformanceMetrics(const FString& OperationName);
    
    // Memory monitoring
    static void LogMemoryUsage(const FString& Context);
    static uint64 GetCurrentMemoryUsage();
    
    // Settings diagnostics
    static void LogSettingsState();
    static void ValidateSettings();
    
    // UI diagnostics
    static void LogUIState(const FString& UIElement, const FString& Action);
    static void LogButtonClick(const FString& ButtonName, bool bSuccess);
    
    // Asset processing diagnostics
    static void LogAssetOperation(const FString& AssetPath, const FString& Operation, bool bSuccess, const FString& Details = TEXT(""));
    
    // Error reporting
    static void LogError(const FString& ErrorMessage, const FString& Context = TEXT(""));
    static void LogWarning(const FString& WarningMessage, const FString& Context = TEXT(""));
    
    // Generate comprehensive report
    static void GenerateDiagnosticReport();
    static void SaveDiagnosticReport(const FString& FilePath);
    
    // Console commands for testing
    static void RegisterConsoleCommands();
    static void UnregisterConsoleCommands();

private:
    static TMap<FString, FPerformanceMetrics> PerformanceTimers;
    static TArray<FString> DiagnosticLog;
    static FString ReportDirectory;
    static bool bInitialized;
    
    // Console command handlers
    static void CmdGenerateReport();
    static void CmdLogMemory();
    static void CmdLogSettings();
    static void CmdClearLogs();
    static void CmdTestPerformance();
    
    // Helper functions
    static FString GetCategoryString(EDiagnosticCategory Category);
    static FString GetTimestamp();
    static FString FormatMemorySize(uint64 Bytes);
    static void WriteToFile(const FString& FilePath, const FString& Content);
};

// Convenience macros for easier logging
#define MAGIC_LOG(Category, Message) FMagicOptimizerDiagnostics::LogMessage(FMagicOptimizerDiagnostics::EDiagnosticCategory::Category, Message)
#define MAGIC_LOG_SCREEN(Category, Message, Duration) FMagicOptimizerDiagnostics::LogMessage(FMagicOptimizerDiagnostics::EDiagnosticCategory::Category, Message, true, Duration)
#define MAGIC_LOG_ERROR(Message, Context) FMagicOptimizerDiagnostics::LogError(Message, Context)
#define MAGIC_LOG_WARNING(Message, Context) FMagicOptimizerDiagnostics::LogWarning(Message, Context)
#define MAGIC_LOG_PERF_START(Operation) FMagicOptimizerDiagnostics::StartPerformanceTimer(Operation)
#define MAGIC_LOG_PERF_END(Operation) FMagicOptimizerDiagnostics::EndPerformanceTimer(Operation)
