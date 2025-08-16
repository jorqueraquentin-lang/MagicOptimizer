/*
  ContentBrowserActions.cpp
  Part of the MagicOptimizer Unreal Engine plugin.
  Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.
*/
#include "ContentBrowserActions.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Modules/ModuleManager.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetRegistryModule.h"

namespace
{
	static FString EnsureObjectPath(const FString& InPath)
	{
		if (InPath.Contains(TEXT(".")))
		{
			return InPath; // already object path
		}
		FString PackageName;
		FString AssetName;
		if (InPath.Split(TEXT("/"), &PackageName, &AssetName, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
		{
			return InPath + TEXT(".") + AssetName;
		}
		return InPath;
	}
}

namespace ContentBrowserActions
{
	void CopyPathToClipboard(const FString& Path)
	{
		FPlatformApplicationMisc::ClipboardCopy(*Path);
	}

	bool SyncToAssetPath(const FString& InPath)
	{
		const FString ObjectPath = EnsureObjectPath(InPath);
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		const FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(*ObjectPath));
		if (!AssetData.IsValid())
		{
			return false;
		}
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		TArray<FAssetData> AssetsToSync;
		AssetsToSync.Add(AssetData);
		ContentBrowserModule.Get().SyncBrowserToAssets(AssetsToSync);
		return true;
	}
}


