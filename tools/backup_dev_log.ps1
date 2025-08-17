# =============================================================================
# Development Log Backup Script
# =============================================================================
# 
# This script creates timestamped backups of the development log
# to prevent accidental loss of development history
#
# Usage: .\tools\backup_dev_log.ps1 [optional: backup directory]
# =============================================================================

param(
    [string]$BackupDir = "docs/backups"
)

# =============================================================================
# CONFIGURATION
# =============================================================================
$DevLogPath = "docs/DEVELOPMENT_LOG.md"
$BackupDir = $BackupDir.TrimEnd('/').TrimEnd('\')
$Timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$BackupFileName = "DEVELOPMENT_LOG_BACKUP_$Timestamp.md"

# =============================================================================
# VALIDATION
# =============================================================================
if (-not (Test-Path $DevLogPath)) {
    Write-Error "❌ Development log not found at: $DevLogPath"
    exit 1
}

# Create backup directory if it doesn't exist
if (-not (Test-Path $BackupDir)) {
    New-Item -ItemType Directory -Path $BackupDir -Force | Out-Null
    Write-Host "✅ Created backup directory: $BackupDir"
}

# =============================================================================
# BACKUP CREATION
# =============================================================================
$BackupPath = Join-Path $BackupDir $BackupFileName

try {
    # Create backup with timestamp header
    $BackupContent = @"
# =============================================================================
# DEVELOPMENT LOG BACKUP - $Timestamp
# =============================================================================
# 
# This is an automatic backup of the development log
# Original file: $DevLogPath
# Backup created: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss UTC")
# 
# WARNING: This is a backup copy. Edit the original file instead.
# =============================================================================

"@

    # Read original content and append to backup
    $OriginalContent = Get-Content $DevLogPath -Raw
    $BackupContent += $OriginalContent

    # Write backup file
    Set-Content -Path $BackupPath -Value $BackupContent -Encoding UTF8

    # Verify backup
    $BackupSize = (Get-Item $BackupPath).Length
    $OriginalSize = (Get-Item $DevLogPath).Length

    if ($BackupSize -ge $OriginalSize) {
        Write-Host "✅ Backup created successfully!"
        Write-Host "   📁 Location: $BackupPath"
        Write-Host "   📊 Size: $($BackupSize / 1KB) KB"
        Write-Host "   🕒 Timestamp: $Timestamp"
        
        # Add to .gitignore if not already there
        $GitIgnorePath = ".gitignore"
        if (Test-Path $GitIgnorePath) {
            $GitIgnoreContent = Get-Content $GitIgnorePath -Raw
            if ($GitIgnoreContent -notmatch "docs/backups/") {
                Add-Content $GitIgnorePath "`n# Development log backups`ndocs/backups/"
                Write-Host "   🔒 Added backup directory to .gitignore"
            }
        }
    } else {
        Write-Error "❌ Backup verification failed - size mismatch"
        exit 1
    }

} catch {
    Write-Error "❌ Backup failed: $($_.Exception.Message)"
    exit 1
}

# =============================================================================
# CLEANUP OLD BACKUPS (keep last 10)
# =============================================================================
try {
    $Backups = Get-ChildItem -Path $BackupDir -Filter "DEVELOPMENT_LOG_BACKUP_*.md" | Sort-Object LastWriteTime -Descending
    if ($Backups.Count -gt 10) {
        $OldBackups = $Backups | Select-Object -Skip 10
        foreach ($OldBackup in $OldBackups) {
            Remove-Item $OldBackup.FullName -Force
            Write-Host "   🗑️  Cleaned up old backup: $($OldBackup.Name)"
        }
    }
} catch {
    Write-Warning "⚠️  Could not clean up old backups: $($_.Exception.Message)"
}

Write-Host "`n🎯 Next steps:"
Write-Host "   1. Edit the development log safely"
Write-Host "   2. Use the safety checklist before committing"
Write-Host "   3. Run this script before major changes"
Write-Host "   4. Check git diff to verify only additions"
