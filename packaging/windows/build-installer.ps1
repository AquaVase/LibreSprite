[CmdletBinding()]
param(
  [Parameter(Mandatory=$true)][string]$SourceDirectory,
  [Parameter(Mandatory=$true)][string]$OutputDirectory,
  [ValidatePattern('^[0-9]+(\.[0-9]+){0,3}$')][string]$Version = '1.2',
  [string]$Compiler = '',
  [ValidatePattern('^[A-Za-z0-9._-]+$')][string]$OutputName = 'LibreSpriteInstaller'
)

$ErrorActionPreference = 'Stop'
$SourceDirectory = (Resolve-Path -LiteralPath $SourceDirectory).Path
foreach ($required in @('libresprite.exe', 'data\gui.xml', 'data\skins\default\skin.xml')) {
  if (!(Test-Path -LiteralPath (Join-Path $SourceDirectory $required) -PathType Leaf)) {
    throw "Missing application file: $required"
  }
}
if (!(Get-ChildItem -LiteralPath $SourceDirectory -Filter '*.dll')) {
  throw 'SourceDirectory must contain the portable application and its runtime DLLs.'
}
if (!$Compiler) {
  $command = Get-Command ISCC.exe -ErrorAction SilentlyContinue
  if ($command) { $Compiler = $command.Source }
  else {
    foreach ($candidate in @("${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe", "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe")) {
      if (Test-Path -LiteralPath $candidate) { $Compiler = $candidate; break }
    }
  }
}
if (!$Compiler -or !(Test-Path -LiteralPath $Compiler -PathType Leaf)) {
  throw 'Install Inno Setup 6.7.1 or later, or supply -Compiler with the path to ISCC.exe.'
}
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
New-Item -ItemType Directory -Force $OutputDirectory | Out-Null
& $Compiler "/DSourceDir=$SourceDirectory" "/DMyAppVersion=$Version" "/DInstallerOutputDir=$OutputDirectory" "/F$OutputName" (Join-Path $PSScriptRoot 'WindowsInstaller.iss')
if ($LASTEXITCODE -ne 0) { throw "Installer compilation failed (exit $LASTEXITCODE)." }
$installer = Join-Path $OutputDirectory "$OutputName.exe"
if (!(Test-Path -LiteralPath $installer)) { throw "Compiler did not create $installer" }
Get-Item -LiteralPath $installer
Get-FileHash -LiteralPath $installer -Algorithm SHA256
