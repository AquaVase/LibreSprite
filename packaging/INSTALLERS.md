# Installers

## Windows x64

The Inno Setup installer installs LibreSprite for the current user without
administrator privileges. It includes the executable, runtime DLLs, application
data and license. It creates a Start menu shortcut and an uninstall entry.
Desktop shortcuts and `.ase` / `.aseprite` Open with entries are optional.
Existing default applications and user preferences are preserved.

Build the portable Windows application first, including its runtime DLLs
(`packaging/windows/package_win.js` collects these in CI). Install
[Inno Setup 6.7.1 or later](https://jrsoftware.org/isdl.php), then run:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File packaging\windows\build-installer.ps1 -SourceDirectory C:\path\portable-LibreSprite -OutputDirectory C:\path\installers -Version 1.2
```

Use `-Compiler C:\path\ISCC.exe` when the compiler is not installed in its
standard location. `-OutputName LibreSprite-1.2-windows-x64-installer` changes
the generated filename. Output can be placed outside the repository.

Run the generated executable to install. Re-running the installer upgrades the
same installation, using its stable application ID. Uninstall through Windows
Settings or `unins000.exe`; user preferences and documents are retained.
Inno Setup also supports `/VERYSILENT /SUPPRESSMSGBOXES /NORESTART` and
`/DIR="C:\path\LibreSprite"` for automated installation.

The Windows workflow builds and uploads an installer on pushes to `master`
and `installer`, pull requests, and release tags. Tagged installers are also
attached to the release. Local builds are unsigned; signing requires a
publisher certificate and is not configured here.

## TODO

- [ ] macOS: signed and notarized DMG or PKG installer, installation and removal checks.
- [ ] Linux: DEB and RPM packages with desktop integration and upgrade/removal checks.
- [ ] Android: APK installation and upgrade validation, distribution instructions.
- [ ] Windows ARM64: native build, dependency packaging and installer validation.
