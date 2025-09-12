#include "MagicOptimizerDiagnostics.h"
#include "MagicOptimizer.h"
#include "HAL/IConsoleManager.h"
#include "Misc/App.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "OptimizerSettings.h"

// Static member definitions
TMap<FString, FMagicOptimizerDiagnostics::FPerformanceMetrics> FMagicOptimizerDiagnostics::PerformanceTimers;
TArray<FString> FMagicOptimizerDiagnostics::DiagnosticLog;
FString FMagicOptimizerDiagnostics::ReportDirectory;
bool FMagicOptimizerDiagnostics::bInitialized = false;

void FMagicOptimizerDiagnostics::Initialize()
{
    if (bInitialized)
    {
        return;
    }
    
    // Create report directory
    ReportDirectory = FPaths::ProjectSavedDir() / TEXT("MagicOptimizer") / TEXT("Diagnostics");
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    PlatformFile.CreateDirectoryTree(*ReportDirectory);
    
    // Register console commands
    RegisterConsoleCommands();
    
    // Log initialization
    LogMessage(EDiagnosticCategory::General, TEXT("MagicOptimizer Diagnostics System Initialized"), true, 3.0f);
    LogMessage(EDiagnosticCategory::General, FString::Printf(TEXT("Report Directory: %s"), *ReportDirectory), false);
    
    bInitialized = true;
}

void FMagicOptimizerDiagnostics::Shutdown()
{
    if (!bInitialized)
    {
        return;
    }
    
    // Generate final report
    GenerateDiagnosticReport();
    
    // Unregister console commands
    UnregisterConsoleCommands();
    
    LogMessage(EDiagnosticCategory::General, TEXT("MagicOptimizer Diagnostics System Shutdown"), true, 3.0f);
    
    bInitialized = false;
}

void FMagicOptimizerDiagnostics::LogMessage(EDiagnosticCategory Category, const FString& Message, bool bOnScreen, float OnScreenDuration)
{
    FString Timestamp = GetTimestamp();
    FString CategoryStr = GetCategoryString(Category);
    FString FullMessage = FString::Printf(TEXT("[%s] [%s] %s"), *Timestamp, *CategoryStr, *Message);
    
    // Add to diagnostic log
    DiagnosticLog.Add(FullMessage);
    
    // Log to console with appropriate verbosity
    switch (Category)
    {
        case EDiagnosticCategory::Error:
            UE_LOG(LogMagicOptimizer, Error, TEXT("%s"), *Message);
            break;
        case EDiagnosticCategory::Warning:
            UE_LOG(LogMagicOptimizer, Warning, TEXT("%s"), *Message);
            break;
        case EDiagnosticCategory::Performance:
            UE_LOG(LogMagicOptimizer, Log, TEXT("[PERF] %s"), *Message);
            break;
        case EDiagnosticCategory::Memory:
            UE_LOG(LogMagicOptimizer, Log, TEXT("[MEM] %s"), *Message);
            break;
        default:
            UE_LOG(LogMagicOptimizer, Log, TEXT("%s"), *Message);
            break;
    }
    
    // Display on screen if requested
    if (bOnScreen && GEngine)
    {
        FColor DisplayColor = FColor::White;
        switch (Category)
        {
            case EDiagnosticCategory::Error:
                DisplayColor = FColor::Red;
                break;
            case EDiagnosticCategory::Warning:
                DisplayColor = FColor::Yellow;
                break;
            case EDiagnosticCategory::Performance:
                DisplayColor = FColor::Cyan;
                break;
            case EDiagnosticCategory::Memory:
                DisplayColor = FColor::Green;
                break;
            default:
                DisplayColor = FColor::White;
                break;
        }
        
        GEngine->AddOnScreenDebugMessage(-1, OnScreenDuration, DisplayColor, FullMessage);
    }
}

void FMagicOptimizerDiagnostics::StartPerformanceTimer(const FString& OperationName)
{
    FPerformanceMetrics Metrics;
    Metrics.OperationName = OperationName;
    Metrics.StartTime = FPlatformTime::Seconds();
    Metrics.MemoryUsage = GetCurrentMemoryUsage();
    
    PerformanceTimers.Add(OperationName, Metrics);
    
    LogMessage(EDiagnosticCategory::Performance, FString::Printf(TEXT("Started performance timer for: %s"), *OperationName));
}

void FMagicOptimizerDiagnostics::EndPerformanceTimer(const FString& OperationName)
{
    if (FPerformanceMetrics* Metrics = PerformanceTimers.Find(OperationName))
    {
        Metrics->EndTime = FPlatformTime::Seconds();
        Metrics->Duration = Metrics->EndTime - Metrics->StartTime;
        
        LogMessage(EDiagnosticCategory::Performance, 
            FString::Printf(TEXT("Performance timer ended for: %s - Duration: %.3f seconds"), 
                *OperationName, Metrics->Duration));
        
        // Log memory usage change
        uint64 CurrentMemory = GetCurrentMemoryUsage();
        int64 MemoryDelta = (int64)CurrentMemory - (int64)Metrics->MemoryUsage;
        LogMessage(EDiagnosticCategory::Memory, 
            FString::Printf(TEXT("Memory delta for %s: %s"), 
                *OperationName, *FormatMemorySize(MemoryDelta)));
    }
    else
    {
        LogMessage(EDiagnosticCategory::Warning, FString::Printf(TEXT("Performance timer not found for: %s"), *OperationName));
    }
}

FMagicOptimizerDiagnostics::FPerformanceMetrics FMagicOptimizerDiagnostics::GetPerformanceMetrics(const FString& OperationName)
{
    if (FPerformanceMetrics* Metrics = PerformanceTimers.Find(OperationName))
    {
        return *Metrics;
    }
    return FPerformanceMetrics();
}

void FMagicOptimizerDiagnostics::LogMemoryUsage(const FString& Context)
{
    uint64 MemoryUsage = GetCurrentMemoryUsage();
    LogMessage(EDiagnosticCategory::Memory, 
        FString::Printf(TEXT("Memory usage %s: %s"), *Context, *FormatMemorySize(MemoryUsage)));
}

uint64 FMagicOptimizerDiagnostics::GetCurrentMemoryUsage()
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical;
}

void FMagicOptimizerDiagnostics::LogSettingsState()
{
    if (UOptimizerSettings* Settings = GetMutableDefault<UOptimizerSettings>())
    {
        LogMessage(EDiagnosticCategory::Settings, TEXT("=== SETTINGS STATE ==="));
        LogMessage(EDiagnosticCategory::Settings, FString::Printf(TEXT("Default Profile: %s"), *Settings->DefaultProfile));
        LogMessage(EDiagnosticCategory::Settings, FString::Printf(TEXT("Enable Auto-Save: %s"), Settings->bEnableAutoSave ? TEXT("True") : TEXT("False")));
        LogMessage(EDiagnosticCategory::Settings, FString::Printf(TEXT("Max Assets Per Batch: %d"), Settings->MaxAssetsPerBatch));
        LogMessage(EDiagnosticCategory::Settings, FString::Printf(TEXT("Process Textures: %s"), Settings->bProcessTextures ? TEXT("True") : TEXT("False")));
        LogMessage(EDiagnosticCategory::Settings, FString::Printf(TEXT("Process Meshes: %s"), Settings->bProcessMeshes ? TEXT("True") : TEXT("False")));
        LogMessage(EDiagnosticCategory::Settings, FString::Printf(TEXT("Process Materials: %s"), Settings->bProcessMaterials ? TEXT("True") : TEXT("False")));
        LogMessage(EDiagnosticCategory::Settings, FString::Printf(TEXT("Create Backups: %s"), Settings->bCreateBackups ? TEXT("True") : TEXT("False")));
        LogMessage(EDiagnosticCategory::Settings, FString::Printf(TEXT("Dry Run by Default: %s"), Settings->bDryRunByDefault ? TEXT("True") : TEXT("False")));
        LogMessage(EDiagnosticCategory::Settings, FString::Printf(TEXT("Max Changes Per Run: %d"), Settings->MaxChangesPerRun));
        LogMessage(EDiagnosticCategory::Settings, FString::Printf(TEXT("Output Directory: %s"), *Settings->OutputDirectory));
        LogMessage(EDiagnosticCategory::Settings, FString::Printf(TEXT("Generate Reports: %s"), Settings->bGenerateReports ? TEXT("True") : TEXT("False")));
        LogMessage(EDiagnosticCategory::Settings, FString::Printf(TEXT("Include Timestamps: %s"), Settings->bIncludeTimestamps ? TEXT("True") : TEXT("False")));
        LogMessage(EDiagnosticCategory::Settings, TEXT("=== END SETTINGS STATE ==="));
    }
    else
    {
        LogMessage(EDiagnosticCategory::Error, TEXT("Settings object not found!"));
    }
}

void FMagicOptimizerDiagnostics::ValidateSettings()
{
    LogMessage(EDiagnosticCategory::Settings, TEXT("Validating settings..."));
    
    if (UOptimizerSettings* Settings = GetMutableDefault<UOptimizerSettings>())
    {
        bool bHasErrors = false;
        
        // Validate MaxAssetsPerBatch
        if (Settings->MaxAssetsPerBatch <= 0 || Settings->MaxAssetsPerBatch > 10000)
        {
            LogMessage(EDiagnosticCategory::Error, FString::Printf(TEXT("Invalid MaxAssetsPerBatch: %d (should be 1-10000)"), Settings->MaxAssetsPerBatch));
            bHasErrors = true;
        }
        
        // Validate MaxChangesPerRun
        if (Settings->MaxChangesPerRun <= 0 || Settings->MaxChangesPerRun > 1000)
        {
            LogMessage(EDiagnosticCategory::Error, FString::Printf(TEXT("Invalid MaxChangesPerRun: %d (should be 1-1000)"), Settings->MaxChangesPerRun));
            bHasErrors = true;
        }
        
        // Validate OutputDirectory
        if (Settings->OutputDirectory.IsEmpty())
        {
            LogMessage(EDiagnosticCategory::Error, TEXT("OutputDirectory is empty"));
            bHasErrors = true;
        }
        
        // Validate DefaultProfile
        if (Settings->DefaultProfile.IsEmpty())
        {
            LogMessage(EDiagnosticCategory::Error, TEXT("DefaultProfile is empty"));
            bHasErrors = true;
        }
        
        if (!bHasErrors)
        {
            LogMessage(EDiagnosticCategory::Settings, TEXT("Settings validation passed"));
        }
    }
    else
    {
        LogMessage(EDiagnosticCategory::Error, TEXT("Settings object not found for validation"));
    }
}

void FMagicOptimizerDiagnostics::LogUIState(const FString& UIElement, const FString& Action)
{
    LogMessage(EDiagnosticCategory::UI, FString::Printf(TEXT("UI %s: %s"), *UIElement, *Action));
}

void FMagicOptimizerDiagnostics::LogButtonClick(const FString& ButtonName, bool bSuccess)
{
    LogMessage(EDiagnosticCategory::UI, 
        FString::Printf(TEXT("Button '%s' clicked - Success: %s"), 
            *ButtonName, bSuccess ? TEXT("True") : TEXT("False")));
}

void FMagicOptimizerDiagnostics::LogAssetOperation(const FString& AssetPath, const FString& Operation, bool bSuccess, const FString& Details)
{
    LogMessage(EDiagnosticCategory::AssetProcessing, 
        FString::Printf(TEXT("Asset Operation: %s on %s - Success: %s - Details: %s"), 
            *Operation, *AssetPath, bSuccess ? TEXT("True") : TEXT("False"), *Details));
}

void FMagicOptimizerDiagnostics::LogError(const FString& ErrorMessage, const FString& Context)
{
    FString FullMessage = Context.IsEmpty() ? ErrorMessage : FString::Printf(TEXT("%s (Context: %s)"), *ErrorMessage, *Context);
    LogMessage(EDiagnosticCategory::Error, FullMessage, true, 10.0f);
}

void FMagicOptimizerDiagnostics::LogWarning(const FString& WarningMessage, const FString& Context)
{
    FString FullMessage = Context.IsEmpty() ? WarningMessage : FString::Printf(TEXT("%s (Context: %s)"), *WarningMessage, *Context);
    LogMessage(EDiagnosticCategory::Warning, FullMessage, true, 7.0f);
}

void FMagicOptimizerDiagnostics::GenerateDiagnosticReport()
{
    LogMessage(EDiagnosticCategory::General, TEXT("Generating comprehensive diagnostic report..."));
    
    FString ReportContent;
    ReportContent += TEXT("=== MAGIC OPTIMIZER DIAGNOSTIC REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n"), *GetTimestamp());
    ReportContent += FString::Printf(TEXT("Report Directory: %s\n\n"), *ReportDirectory);
    
    // System information
    ReportContent += TEXT("=== SYSTEM INFORMATION ===\n");
    ReportContent += FString::Printf(TEXT("Platform: %hs\n"), FPlatformProperties::PlatformName());
    ReportContent += FString::Printf(TEXT("Engine Version: %s\n"), *FEngineVersion::Current().ToString());
    ReportContent += FString::Printf(TEXT("Memory Usage: %s\n"), *FormatMemorySize(GetCurrentMemoryUsage()));
    ReportContent += TEXT("\n");
    
    // Performance metrics
    ReportContent += TEXT("=== PERFORMANCE METRICS ===\n");
    for (const auto& Pair : PerformanceTimers)
    {
        const FPerformanceMetrics& Metrics = Pair.Value;
        ReportContent += FString::Printf(TEXT("Operation: %s\n"), *Metrics.OperationName);
        ReportContent += FString::Printf(TEXT("  Duration: %.3f seconds\n"), Metrics.Duration);
        ReportContent += FString::Printf(TEXT("  Memory Usage: %s\n"), *FormatMemorySize(Metrics.MemoryUsage));
        ReportContent += FString::Printf(TEXT("  Asset Count: %d\n"), Metrics.AssetCount);
        ReportContent += TEXT("\n");
    }
    
    // Diagnostic log
    ReportContent += TEXT("=== DIAGNOSTIC LOG ===\n");
    for (const FString& LogEntry : DiagnosticLog)
    {
        ReportContent += LogEntry + TEXT("\n");
    }
    
    // Save report
    FString ReportPath = ReportDirectory / FString::Printf(TEXT("DiagnosticReport_%s.txt"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
    SaveDiagnosticReport(ReportPath);
    
    LogMessage(EDiagnosticCategory::General, FString::Printf(TEXT("Diagnostic report saved to: %s"), *ReportPath));
}

void FMagicOptimizerDiagnostics::SaveDiagnosticReport(const FString& FilePath)
{
    FString ReportContent;
    ReportContent += TEXT("=== MAGIC OPTIMIZER DIAGNOSTIC REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n"), *GetTimestamp());
    ReportContent += FString::Printf(TEXT("Report Directory: %s\n\n"), *ReportDirectory);
    
    // System information
    ReportContent += TEXT("=== SYSTEM INFORMATION ===\n");
    ReportContent += FString::Printf(TEXT("Platform: %hs\n"), FPlatformProperties::PlatformName());
    ReportContent += FString::Printf(TEXT("Engine Version: %s\n"), *FEngineVersion::Current().ToString());
    ReportContent += FString::Printf(TEXT("Memory Usage: %s\n"), *FormatMemorySize(GetCurrentMemoryUsage()));
    ReportContent += TEXT("\n");
    
    // Performance metrics
    ReportContent += TEXT("=== PERFORMANCE METRICS ===\n");
    for (const auto& Pair : PerformanceTimers)
    {
        const FPerformanceMetrics& Metrics = Pair.Value;
        ReportContent += FString::Printf(TEXT("Operation: %s\n"), *Metrics.OperationName);
        ReportContent += FString::Printf(TEXT("  Duration: %.3f seconds\n"), Metrics.Duration);
        ReportContent += FString::Printf(TEXT("  Memory Usage: %s\n"), *FormatMemorySize(Metrics.MemoryUsage));
        ReportContent += FString::Printf(TEXT("  Asset Count: %d\n"), Metrics.AssetCount);
        ReportContent += TEXT("\n");
    }
    
    // Diagnostic log
    ReportContent += TEXT("=== DIAGNOSTIC LOG ===\n");
    for (const FString& LogEntry : DiagnosticLog)
    {
        ReportContent += LogEntry + TEXT("\n");
    }
    
    WriteToFile(FilePath, ReportContent);
}

void FMagicOptimizerDiagnostics::RegisterConsoleCommands()
{
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("MagicOptimizer.GenerateReport"),
        TEXT("Generate a comprehensive diagnostic report"),
        FConsoleCommandDelegate::CreateStatic(&CmdGenerateReport)
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("MagicOptimizer.LogMemory"),
        TEXT("Log current memory usage"),
        FConsoleCommandDelegate::CreateStatic(&CmdLogMemory)
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("MagicOptimizer.LogSettings"),
        TEXT("Log current settings state"),
        FConsoleCommandDelegate::CreateStatic(&CmdLogSettings)
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("MagicOptimizer.ClearLogs"),
        TEXT("Clear diagnostic logs"),
        FConsoleCommandDelegate::CreateStatic(&CmdClearLogs)
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("MagicOptimizer.TestPerformance"),
        TEXT("Test performance monitoring"),
        FConsoleCommandDelegate::CreateStatic(&CmdTestPerformance)
    );
}

void FMagicOptimizerDiagnostics::UnregisterConsoleCommands()
{
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("MagicOptimizer.GenerateReport"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("MagicOptimizer.LogMemory"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("MagicOptimizer.LogSettings"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("MagicOptimizer.ClearLogs"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("MagicOptimizer.TestPerformance"));
}

void FMagicOptimizerDiagnostics::CmdGenerateReport()
{
    GenerateDiagnosticReport();
}

void FMagicOptimizerDiagnostics::CmdLogMemory()
{
    LogMemoryUsage(TEXT("Console Command"));
}

void FMagicOptimizerDiagnostics::CmdLogSettings()
{
    LogSettingsState();
}

void FMagicOptimizerDiagnostics::CmdClearLogs()
{
    DiagnosticLog.Empty();
    PerformanceTimers.Empty();
    LogMessage(EDiagnosticCategory::General, TEXT("Diagnostic logs cleared"));
}

void FMagicOptimizerDiagnostics::CmdTestPerformance()
{
    LogMessage(EDiagnosticCategory::General, TEXT("Testing performance monitoring..."));
    
    StartPerformanceTimer(TEXT("TestOperation"));
    
    // Simulate some work
    FPlatformProcess::Sleep(0.1f);
    
    EndPerformanceTimer(TEXT("TestOperation"));
    
    LogMessage(EDiagnosticCategory::General, TEXT("Performance test completed"));
}

FString FMagicOptimizerDiagnostics::GetCategoryString(EDiagnosticCategory Category)
{
    switch (Category)
    {
        case EDiagnosticCategory::General: return TEXT("GENERAL");
        case EDiagnosticCategory::Performance: return TEXT("PERFORMANCE");
        case EDiagnosticCategory::Memory: return TEXT("MEMORY");
        case EDiagnosticCategory::Settings: return TEXT("SETTINGS");
        case EDiagnosticCategory::UI: return TEXT("UI");
        case EDiagnosticCategory::AssetProcessing: return TEXT("ASSET");
        case EDiagnosticCategory::Error: return TEXT("ERROR");
        case EDiagnosticCategory::Warning: return TEXT("WARNING");
        case EDiagnosticCategory::Info: return TEXT("INFO");
        default: return TEXT("UNKNOWN");
    }
}

FString FMagicOptimizerDiagnostics::GetTimestamp()
{
    return FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S.%f"));
}

FString FMagicOptimizerDiagnostics::FormatMemorySize(uint64 Bytes)
{
    if (Bytes < 1024)
    {
        return FString::Printf(TEXT("%llu B"), Bytes);
    }
    else if (Bytes < 1024 * 1024)
    {
        return FString::Printf(TEXT("%.2f KB"), Bytes / 1024.0);
    }
    else if (Bytes < 1024 * 1024 * 1024)
    {
        return FString::Printf(TEXT("%.2f MB"), Bytes / (1024.0 * 1024.0));
    }
    else
    {
        return FString::Printf(TEXT("%.2f GB"), Bytes / (1024.0 * 1024.0 * 1024.0));
    }
}

void FMagicOptimizerDiagnostics::WriteToFile(const FString& FilePath, const FString& Content)
{
    FFileHelper::SaveStringToFile(Content, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get());
}
