# =============================================================================
# Development Log Validation Script
# =============================================================================
# 
# This script validates that development log changes are safe
# by checking that we're only adding content, not deleting
#
# Usage: .\tools\validate_dev_log.ps1
# =============================================================================

# =============================================================================
# CONFIGURATION
# =============================================================================
$DevLogPath = "docs/DEVELOPMENT_LOG.md"

# =============================================================================
# VALIDATION CHECKS
# =============================================================================
Write-Host "Validating Development Log Safety..." -ForegroundColor Cyan

# Check 1: File exists
if (-not (Test-Path $DevLogPath)) {
    Write-Error "ERROR: Development log not found at: $DevLogPath"
    exit 1
}

# Check 2: File size (should be substantial)
$FileSize = (Get-Item $DevLogPath).Length
$FileSizeKB = [math]::Round($FileSize / 1KB, 2)

if ($FileSize -lt 1000) {
    Write-Warning "WARNING: Development log seems very small ($FileSizeKB KB) - possible corruption?"
} else {
    Write-Host "SUCCESS: File size: $FileSizeKB KB" -ForegroundColor Green
}

# Check 3: Contains expected content
$Content = Get-Content $DevLogPath -Raw
$PhaseCount = ([regex]::Matches($Content, "### .*Phase \d+:")).Count

if ($PhaseCount -eq 0) {
    Write-Error "ERROR: No development phases found - possible corruption!"
    exit 1
} else {
    Write-Host "SUCCESS: Found $PhaseCount development phases" -ForegroundColor Green
}

# Check 4: Contains safety warnings
if ($Content -match "CRITICAL SAFETY WARNINGS") {
    Write-Host "SUCCESS: Safety warnings present" -ForegroundColor Green
} else {
    Write-Warning "WARNING: Safety warnings missing - consider adding them"
}

# Check 5: Git status check
Write-Host "`nChecking Git Status..." -ForegroundColor Cyan

try {
    $GitStatus = git status --porcelain
    if ($GitStatus) {
        Write-Host "Git changes detected:" -ForegroundColor Yellow
        $GitStatus | ForEach-Object { Write-Host "   $_" -ForegroundColor Yellow }
        
        # Check if development log is modified
        if ($GitStatus -match "DEVELOPMENT_LOG\.md") {
            Write-Host "`nAnalyzing Development Log Changes..." -ForegroundColor Cyan
            
            # Get diff for development log
            $Diff = git diff docs/DEVELOPMENT_LOG.md
            
            if ($Diff) {
                $AddedLines = ($Diff | Select-String "^\+" | Where-Object { $_.Line -notmatch "^\+\+\+" }).Count
                $RemovedLines = ($Diff | Select-String "^-" | Where-Object { $_.Line -notmatch "^---" }).Count
                
                Write-Host "   Changes Summary:" -ForegroundColor White
                Write-Host "      Lines Added: $AddedLines" -ForegroundColor Green
                Write-Host "      Lines Removed: $RemovedLines" -ForegroundColor Red
                
                if ($RemovedLines -gt 0) {
                    Write-Warning "WARNING: Lines were removed from development log!"
                    Write-Warning "   This could indicate accidental deletion of content."
                    Write-Warning "   Please review the changes carefully before committing."
                    
                    # Show removed lines
                    Write-Host "`n   REMOVED Content:" -ForegroundColor Red
                    $Diff | Select-String "^-" | Where-Object { $_.Line -notmatch "^---" } | ForEach-Object {
                        Write-Host "      $($_.Line)" -ForegroundColor Red
                    }
                } else {
                    Write-Host "SUCCESS: Only additions detected - safe to commit" -ForegroundColor Green
                }
                
                if ($AddedLines -gt 0) {
                    Write-Host "`n   ADDED Content:" -ForegroundColor Green
                    $Diff | Select-String "^\+" | Where-Object { $_.Line -notmatch "^\+\+\+" } | Select-Object -First 10 | ForEach-Object {
                        Write-Host "      $($_.Line)" -ForegroundColor Green
                    }
                    if ($AddedLines -gt 10) {
                        Write-Host "      ... and $($AddedLines - 10) more lines" -ForegroundColor Green
                    }
                }
            } else {
                Write-Host "SUCCESS: No changes detected in development log" -ForegroundColor Green
            }
        }
    } else {
        Write-Host "SUCCESS: Working tree is clean" -ForegroundColor Green
    }
} catch {
    Write-Warning "WARNING: Could not check git status: $($_.Exception.Message)"
}

# Check 6: Backup recommendation
Write-Host "`nSafety Recommendations:" -ForegroundColor Cyan

if ($GitStatus -match "DEVELOPMENT_LOG\.md") {
    Write-Host "   Before committing:" -ForegroundColor White
    Write-Host "      1. Run: .\tools\backup_dev_log.ps1" -ForegroundColor Yellow
    Write-Host "      2. Review git diff carefully" -ForegroundColor Yellow
    Write-Host "      3. Ensure only additions, no deletions" -ForegroundColor Yellow
    Write-Host "      4. Use the safety checklist in the log" -ForegroundColor Yellow
} else {
    Write-Host "   SUCCESS: No development log changes to commit" -ForegroundColor Green
}

Write-Host "`nValidation Complete!" -ForegroundColor Cyan
