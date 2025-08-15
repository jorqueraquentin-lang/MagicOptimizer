#pragma once

#include "CoreMinimal.h"

namespace ContentBrowserActions
{
	// Copies the provided path to the system clipboard
	void CopyPathToClipboard(const FString& Path);

	// Attempts to find and sync the Content Browser to the asset at the given path.
	// Accepts package path ("/Game/Path/Asset") or object path ("/Game/Path/Asset.Asset").
	// Returns true on success.
	bool SyncToAssetPath(const FString& InPath);
}


