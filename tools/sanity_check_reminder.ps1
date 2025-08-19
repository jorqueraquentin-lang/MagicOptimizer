<#
  sanity_check_reminder.ps1
  Part of the MagicOptimizer Unreal Engine plugin.
  Copyright (c) 2025 Perseus XR PTY LTD. All rights reserved.
  
  This script helps remind developers to perform regular project sanity checks
  and tracks when checks are due.
#>

param(
    [switch]$Check,
    [switch]$Daily,
    [switch]$Weekly,
    [switch]$Monthly,
    [switch]$ShowStatus
)

$ErrorActionPreference = "Stop"

# Paths
$RepoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path | Split-Path -Parent
$DevLog = Join-Path $RepoRoot "docs/DEVELOPMENT_LOG.md"
$SanityChecksFolder = Join-Path $RepoRoot "docs/sanity_checks"
$LastCheckFile = Join-Path $RepoRoot "tools/.last_sanity_check"

# Colors for output
$Green = "Green"
$Yellow = "Yellow" 
$Red = "Red"
$Cyan = "Cyan"

function Write-ColorOutput {
    param([string]$Message, [string]$Color = "White")
    
    # Map color names to ConsoleColor values
    $colorMap = @{
        "Green" = "Green"
        "Yellow" = "Yellow" 
        "Red" = "Red"
        "Cyan" = "Cyan"
        "White" = "White"
    }
    
    $consoleColor = $colorMap[$Color]
    if (-not $consoleColor) {
        $consoleColor = "White"
    }
    
    Write-Host $Message -ForegroundColor $consoleColor
}

function Get-LastSanityCheck {
    if (Test-Path $LastCheckFile) {
        $content = Get-Content $LastCheckFile -Raw
        try {
            return [DateTime]::Parse($content.Trim())
        } catch {
            return $null
        }
    }
    return $null
}

function Set-LastSanityCheck {
    param([DateTime]$CheckTime)
    $CheckTime.ToString("yyyy-MM-dd HH:mm:ss") | Out-File $LastCheckFile -Encoding UTF8
}

function Get-DevLogLastCheck {
    if (Test-Path $DevLog) {
        $content = Get-Content $DevLog -Raw
        $matches = [regex]::Matches($content, "### SANITY CHECK - (\d{4}-\d{2}-\d{2} \d{2}:\d{2})")
        if ($matches.Count -gt 0) {
            $lastMatch = $matches[$matches.Count - 1]
            try {
                return [DateTime]::Parse($lastMatch.Groups[1].Value)
            } catch {
                return $null
            }
        }
    }
    return $null
}

function Get-SanityChecksFolderLastCheck {
    if (Test-Path $SanityChecksFolder) {
        $files = Get-ChildItem -Path $SanityChecksFolder -Filter "sanity_check_*.md" | Sort-Object LastWriteTime -Descending
        if ($files.Count -gt 0) {
            $lastFile = $files[0]
            $filename = $lastFile.Name
            $match = [regex]::Match($filename, "sanity_check_(\d{4})_(\d{2})_(\d{2})_(\d{2})(\d{2})")
            if ($match.Success) {
                try {
                    $dateStr = "$($match.Groups[1].Value)-$($match.Groups[2].Value)-$($match.Groups[3].Value) $($match.Groups[4].Value):$($match.Groups[5].Value)"
                    return [DateTime]::Parse($dateStr)
                } catch {
                    return $null
                }
            }
        }
    }
    return $null
}

function Show-SanityCheckStatus {
    $lastCheck = Get-LastSanityCheck
    $lastLogCheck = Get-DevLogLastCheck
    $lastSanityCheckFile = Get-SanityChecksFolderLastCheck
    $now = Get-Date
    
    Write-ColorOutput "`nPROJECT SANITY CHECK STATUS" $Cyan
    Write-ColorOutput "=================================" $Cyan
    
    if ($lastCheck) {
        $dailyDiff = $now - $lastCheck
        $weeklyDiff = $now - $lastCheck
        $monthlyDiff = $now - $lastCheck
        
        Write-ColorOutput "Last Sanity Check: $($lastCheck.ToString('yyyy-MM-dd HH:mm:ss'))" $White
        
        # Daily check status
        if ($dailyDiff.TotalHours -le 24) {
            Write-ColorOutput "Daily Check: DONE (within 24h)" $Green
        } else {
            Write-ColorOutput "Daily Check: OVERDUE ($([math]::Floor($dailyDiff.TotalHours))h ago)" $Red
        }
        
        # Weekly check status
        if ($weeklyDiff.TotalDays -le 7) {
            Write-ColorOutput "Weekly Check: DONE (within 7 days)" $Green
        } else {
            Write-ColorOutput "Weekly Check: OVERDUE ($([math]::Floor($weeklyDiff.TotalDays)) days ago)" $Red
        }
        
        # Monthly check status
        if ($monthlyDiff.TotalDays -le 30) {
            Write-ColorOutput "Monthly Check: DONE (within 30 days)" $Green
        } else {
            Write-ColorOutput "Monthly Check: OVERDUE ($([math]::Floor($monthlyDiff.TotalDays)) days ago)" $Red
        }
    } else {
        Write-ColorOutput "Last Sanity Check: NEVER PERFORMED" $Red
        Write-ColorOutput "Daily Check: OVERDUE" $Red
        Write-ColorOutput "Weekly Check: OVERDUE" $Red
        Write-ColorOutput "Monthly Check: OVERDUE" $Red
    }
    
    if ($lastLogCheck) {
        Write-ColorOutput "`nLast Logged Check: $($lastLogCheck.ToString('yyyy-MM-dd HH:mm:ss'))" $White
    } else {
        Write-ColorOutput "`nLast Logged Check: NOT FOUND IN DEV LOG" $Red
    }
    
    if ($lastSanityCheckFile) {
        Write-ColorOutput "Last Sanity Check File: $($lastSanityCheckFile.ToString('yyyy-MM-dd HH:mm:ss'))" $White
    } else {
        Write-ColorOutput "Last Sanity Check File: NOT FOUND IN SANITY_CHECKS FOLDER" $Red
    }
    
    Write-ColorOutput "`nNEXT ACTIONS:" $Cyan
    Write-ColorOutput "1. Perform sanity check if overdue" $White
    Write-ColorOutput "2. Create timestamped file in docs/sanity_checks/ folder" $White
    Write-ColorOutput "3. Add summary entry to docs/DEVELOPMENT_LOG.md" $White
    Write-ColorOutput "4. Update project priorities based on findings" $White
}

function Start-SanityCheck {
    param([string]$CheckType)
    
    $now = Get-Date
    Write-ColorOutput "`nSTARTING $CheckType SANITY CHECK" $Cyan
    Write-ColorOutput "Time: $($now.ToString('yyyy-MM-dd HH:mm:ss'))" $White
    Write-ColorOutput "=================================" $Cyan
    
    Write-ColorOutput "`nPREPARATION (2-3 minutes):" $Yellow
    Write-ColorOutput "- Review last sanity check log entry" $White
    Write-ColorOutput "- Check current development log for recent progress" $White
    Write-ColorOutput "- Review project roadmap and milestones" $White
    
    Write-ColorOutput "`nASSESSMENT (5-15 minutes):" $Yellow
    Write-ColorOutput "- Answer the relevant check questions" $White
    Write-ColorOutput "- Compare current state to planned objectives" $White
    Write-ColorOutput "- Identify any misalignments or issues" $White
    
    Write-ColorOutput "`nACTION PLANNING (3-5 minutes):" $Yellow
    Write-ColorOutput "- Define specific next actions" $White
    Write-ColorOutput "- Assign responsibility and timelines" $White
    Write-ColorOutput "- Update development priorities if needed" $White
    
    Write-ColorOutput "`nLOGGING (2-3 minutes):" $Yellow
    Write-ColorOutput "- Create timestamped file in docs/sanity_checks/ folder" $White
    Write-ColorOutput "- Document the sanity check results" $White
    Write-ColorOutput "- Add summary entry to docs/DEVELOPMENT_LOG.md" $White
    Write-ColorOutput "- Commit both files to version control" $White
    
    Write-ColorOutput "`nCOMPLETE THE CHECK:" $Green
    Write-ColorOutput "1. Follow the workflow above" $White
    Write-ColorOutput "2. Create timestamped file in docs/sanity_checks/ folder" $White
    Write-ColorOutput "3. Add summary entry to docs/DEVELOPMENT_LOG.md" $White
    Write-ColorOutput "4. Run this script again to mark as complete" $White
    
    Write-ColorOutput "`nTIP: Use the format from .cursor/rules/project-sanity-check.always.mdc" $Cyan
}

function Complete-SanityCheck {
    param([string]$CheckType)
    
    $now = Get-Date
    Set-LastSanityCheck $now
    
    Write-ColorOutput "`nSANITY CHECK COMPLETED" $Green
    Write-ColorOutput "Type: $CheckType" $White
    Write-ColorOutput "Time: $($now.ToString('yyyy-MM-dd HH:mm:ss'))" $White
    Write-ColorOutput "=================================" $Green
    
    Write-ColorOutput "`nREMINDER:" $Yellow
    Write-ColorOutput "- Create timestamped file in docs/sanity_checks/ folder" $White
    Write-ColorOutput "- Add summary entry to docs/DEVELOPMENT_LOG.md" $White
    Write-ColorOutput "- Use the standard format from the rules" $White
    Write-ColorOutput "- Commit both files to version control" $White
    
    Write-ColorOutput "`nNext check due:" $Cyan
    switch ($CheckType) {
        "Daily" { Write-ColorOutput "Within 24 hours" $White }
        "Weekly" { Write-ColorOutput "Within 7 days" $White }
        "Monthly" { Write-ColorOutput "Within 30 days" $White }
    }
}

# Main execution
if ($ShowStatus) {
    Show-SanityCheckStatus
} elseif ($Daily) {
    Start-SanityCheck "DAILY"
} elseif ($Weekly) {
    Start-SanityCheck "WEEKLY"
} elseif ($Monthly) {
    Start-SanityCheck "MONTHLY"
} elseif ($Check) {
    $lastCheck = Get-LastSanityCheck
    $now = Get-Date
    
    if ($lastCheck) {
        $dailyDiff = $now - $lastCheck
        $weeklyDiff = $now - $lastCheck
        $monthlyDiff = $now - $lastCheck
        
        if ($dailyDiff.TotalHours -gt 24) {
            Write-ColorOutput "Daily check overdue! Starting daily sanity check..." $Red
            Start-SanityCheck "DAILY"
        } elseif ($weeklyDiff.TotalDays -gt 7) {
            Write-ColorOutput "Weekly check overdue! Starting weekly sanity check..." $Red
            Start-SanityCheck "WEEKLY"
        } elseif ($monthlyDiff.TotalDays -gt 30) {
            Write-ColorOutput "Monthly check overdue! Starting monthly sanity check..." $Red
            Start-SanityCheck "MONTHLY"
        } else {
            Write-ColorOutput "All sanity checks are up to date!" $Green
            Show-SanityCheckStatus
        }
    } else {
        Write-ColorOutput "No sanity checks performed yet! Starting daily sanity check..." $Red
        Start-SanityCheck "DAILY"
    }
} else {
    Write-ColorOutput "`nMAGICOPTIMIZER SANITY CHECK REMINDER" $Cyan
    Write-ColorOutput "=========================================" $Cyan
    
    Write-ColorOutput "`nUSAGE:" $Yellow
    Write-ColorOutput "  .\sanity_check_reminder.ps1 -Check          # Check if sanity check is due" $White
    Write-ColorOutput "  .\sanity_check_reminder.ps1 -ShowStatus     # Show current status" $White
    Write-ColorOutput "  .\sanity_check_reminder.ps1 -Daily          # Start daily sanity check" $White
    Write-ColorOutput "  .\sanity_check_reminder.ps1 -Weekly         # Start weekly sanity check" $White
    Write-ColorOutput "  .\sanity_check_reminder.ps1 -Monthly        # Start monthly sanity check" $White
    
    Write-ColorOutput "`nFREQUENCY:" $Yellow
    Write-ColorOutput "  Daily: MANDATORY - Every calendar day (5-10 minutes)" $White
    Write-ColorOutput "  Weekly: Every 7 days (15-30 minutes)" $White
    Write-ColorOutput "  Monthly: Every 30 days (30-60 minutes)" $White
    
    Write-ColorOutput "`nLOGGING:" $Yellow
    Write-ColorOutput "  Daily checks: Create timestamped file in docs/sanity_checks/ folder" $White
    Write-ColorOutput "  All checks: Add summary entry to docs/DEVELOPMENT_LOG.md" $White
    Write-ColorOutput "  Use the format from .cursor/rules/project-sanity-check.always.mdc" $White
    
    Write-ColorOutput "`nTIP: Run -Check to see if you're due for a sanity check!" $Cyan
}
