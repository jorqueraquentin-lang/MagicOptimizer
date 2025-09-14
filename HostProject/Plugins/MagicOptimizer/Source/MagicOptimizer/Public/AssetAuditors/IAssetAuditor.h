#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "Model/AuditConfig.h"
#include "Model/AuditResult.h"

/**
 * Abstract base class for asset auditors
 * Defines the interface that all asset type auditors must implement
 */
class MAGICOPTIMIZER_API IAssetAuditor
{
public:
    virtual ~IAssetAuditor() = default;

    /**
     * Check if this auditor can handle the given asset
     * @param AssetData Asset data to check
     * @return True if this auditor can handle the asset
     */
    virtual bool CanAuditAsset(const FAssetData& AssetData) const = 0;

    /**
     * Audit an asset synchronously
     * @param AssetData Asset data to audit
     * @return Audit result
     */
    virtual FAuditResult AuditAsset(const FAssetData& AssetData) const = 0;

    /**
     * Audit an asset asynchronously
     * @param AssetData Asset data to audit
     * @return Future containing audit result
     */
    virtual TFuture<FAuditResult> AuditAssetAsync(const FAssetData& AssetData) const = 0;

    /**
     * Set audit configuration
     * @param Config Audit configuration
     */
    virtual void SetAuditConfig(const FAuditConfig& Config) = 0;

    /**
     * Get current audit configuration
     * @return Current audit configuration
     */
    virtual FAuditConfig GetAuditConfig() const = 0;

    /**
     * Get audit progress (0.0 = not started, 1.0 = completed)
     * @return Audit progress
     */
    virtual float GetAuditProgress() const = 0;

    /**
     * Get current audit status
     * @return Current audit status
     */
    virtual FString GetAuditStatus() const = 0;

    /**
     * Handle an error during audit
     * @param ErrorMessage Error message
     * @return True if error was handled successfully
     */
    virtual bool HandleError(const FString& ErrorMessage) = 0;

    /**
     * Reset auditor to initial state
     */
    virtual void Reset() = 0;

    /**
     * Get auditor name
     * @return Auditor name
     */
    virtual FString GetAuditorName() const = 0;

    /**
     * Get supported asset types
     * @return Array of supported asset types
     */
    virtual TArray<EAssetType> GetSupportedAssetTypes() const = 0;
};
