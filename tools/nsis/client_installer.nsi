; NSIS installer script to make Atrinik client win32 installer.
; To make the installer, place this script in your client directory and
; run `makensis client_installer.nsi`

; Installer name.
Name "Atrinik Client 0.1"

; Installer filename.
OutFile "atrinik-client-0.1.exe"

; Default installation directory.
InstallDir "$PROGRAMFILES\Atrinik Client 0.1"

; Registry key.
InstallDirRegKey HKLM "Software\Atrinik-Client-0.1" "Install_Dir"

; Pages.
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

; What to install.
Section "Client (required)"
  SectionIn RO

  SetOutPath $INSTDIR
  File "atrinik.exe"
  File "atrinik.p0"
  File "*.dll"
  File "License"
  File "*.dat"
  File "README.txt"
  File "scripts_autoload"

  CreateDirectory $INSTDIR\bitmaps
  SetOutPath $INSTDIR\bitmaps
  File "bitmaps\*.*"

  CreateDirectory $INSTDIR\cache
  SetOutPath $INSTDIR\cache
  File "cache\*.*"

  CreateDirectory $INSTDIR\gfx_user
  SetOutPath $INSTDIR\gfx_user
  File "gfx_user\*.*"

  CreateDirectory $INSTDIR\icons
  SetOutPath $INSTDIR\icons
  File "icons\*.*"

  CreateDirectory $INSTDIR\media
  SetOutPath $INSTDIR\media
  File "media\*.*"

  CreateDirectory $INSTDIR\sfx
  SetOutPath $INSTDIR\sfx
  File "sfx\*.*"

  CreateDirectory $INSTDIR\srv_files
  SetOutPath $INSTDIR\srv_files
  File "srv_files\*.*"

  SetOutPath $INSTDIR

  WriteRegStr HKLM SOFTWARE\Atrinik-Client-0.1 "Install_Dir" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Atrinik-Client-0.1" "DisplayName" "Atrinik Client"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Atrinik-Client-0.1" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Atrinik-Client-0.1" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Atrinik-Client-0.1" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
SectionEnd

; Optional start menu shortcuts.
Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\Atrinik Client 0.1"
  CreateShortCut "$SMPROGRAMS\Atrinik Client 0.1\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Atrinik Client 0.1\Atrinik Client.lnk" "$INSTDIR\atrinik.exe" "" "$INSTDIR\atrinik.exe" 0
SectionEnd

; Uninstaller.
Section "Uninstall"
  ; Remove registry keys.
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Atrinik-Client-0.1"
  DeleteRegKey HKLM SOFTWARE\Atrinik-Client-0.1

  RMDir /r /REBOOTOK "$SMPROGRAMS\Atrinik Client 0.1"
  RMDir /r /REBOOTOK "$INSTDIR"
SectionEnd
