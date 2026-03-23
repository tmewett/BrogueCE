<#
  .SYNOPSIS
  Downloads the Git modules specified in ../.gitmodules

  .DESCRIPTION
  Parses and downloads the Github repositories specified in the .gitmodules file

  .EXAMPLE
  PS> .\Get-GitModules.ps1
  < Downloads and parses the repositories in the .gitmodules file. >
#>

#------- Variables -------------------------------------------------------------
[String] $PathRegex   = "path\s*=\s*(?<path>.*)"
[String] $URLRegex    = "url\s*=\s*(?<url>.*)" 
[String] $BranchRegex = "branch\s*=\s*(?<Branch>.*)"
[String[]] $Arguments = $args

#------- Script ----------------------------------------------------------------
if (-not $Arguments) {
    [String[]]$Arguments = "--depth", "1"
}

foreach ($Line in Get-Content $PSScriptRoot\..\.gitmodules) {
    if ($Line -match $PathRegex) {
        $Match  = Select-String -InputObject $Line -Pattern $PathRegex
        $Path   = $Match.Matches[0].Groups[1].Value
    }
    elseif ($Line -match $URLRegex) {
        $Match  = Select-String -InputObject $Line -Pattern $URLRegex
        $URL    = $Match.Matches[0].Groups[1].Value
    }
    elseif ($Line -match $BranchRegex) {
        $Match  = Select-String -InputObject $Line -Pattern $BranchRegex
        $Branch = $Match.Matches[0].Groups[1].Value
        
        Write-Host "git clone --filter=blob:none $URL $Path -b $Branch --recursive $Arguments" `
            -ForegroundColor Blue
        git clone --filter=blob:none $URL $PSScriptRoot/../$Path -b $Branch --recursive $Arguments
    }
}
