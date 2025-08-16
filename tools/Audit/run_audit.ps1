param([switch]$Verbose)
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
python "$here/audit_repo.py" @('--verbose')[$Verbose.IsPresent]

