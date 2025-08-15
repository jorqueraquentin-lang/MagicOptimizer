/*
  OptimizerLogging.cpp
  Part of the MagicOptimizer Unreal Engine plugin.
  Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.
*/
#include "OptimizerLogging.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

namespace MagicOptimizerLog
{
	static FString GetTimestamp()
	{
		return FDateTime::Now().ToString(TEXT("yyyy-MM-dd HH:mm:ss"));
	}

	FString GetLogFilePath()
	{
		const FString Dir = FPaths::ProjectSavedDir() / TEXT("MagicOptimizer");
		return Dir / TEXT("MagicOptimizerRuntime.log");
	}

	FString GetBacklogFilePath()
	{
		const FString Dir = FPaths::ProjectSavedDir() / TEXT("MagicOptimizer");
		return Dir / TEXT("MagicOptimizerBacklog.log");
	}

	void AppendLine(const FString& Line)
	{
		const FString FilePath = GetLogFilePath();
		IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
		PF.CreateDirectoryTree(*FPaths::GetPath(FilePath));
		const FString WithTs = FString::Printf(TEXT("[%s] %s\n"), *GetTimestamp(), *Line);
		FFileHelper::SaveStringToFile(WithTs, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
	}

	void AppendBacklog(const FString& Line)
	{
		const FString FilePath = GetBacklogFilePath();
		IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
		PF.CreateDirectoryTree(*FPaths::GetPath(FilePath));
		const FString WithTs = FString::Printf(TEXT("[%s] %s\n"), *GetTimestamp(), *Line);
		FFileHelper::SaveStringToFile(WithTs, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
	}
}


