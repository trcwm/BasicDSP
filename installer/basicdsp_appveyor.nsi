; basicdsp.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install example2.nsi into a directory that the user selects,

;--------------------------------

!include x64.nsh

; The name of the installer
Name "BasicDSP"

; The file to write
OutFile "basicdsp_installer_x64.exe"

; The default installation directory
InstallDir $PROGRAMFILES64\BasicDSP

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\MoseleyInstruments\BasicDSP" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Install BasicDSP (required)"

  SectionIn RO
  
  ;${If} ${RunningX64}
  ;  ReadRegStr $1 HKLM "SOFTWARE\Microsoft\VisualStudio\10.0\VC\VCRedist\x64" "Installed"
  ;  StrCmp $1 1 installed
  ;${Else}
  ;  ReadRegStr $1 HKLM "SOFTWARE\Microsoft\VisualStudio\10.0\VC\VCRedist\x86" "Installed"
  ;  StrCmp $1 1 installed
  ;${EndIf}

  ;not installed, so run the installer
  ;ExecWait "$INSTDIR\vcredist_x64.exe"

  ;installed:
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "C:\projects\basicdsp\release\BasicDSP.exe"
  File "C:\Qt\5.11.1\msvc2017_64\bin\Qt5Core.dll"
  File "C:\Qt\5.11.1\msvc2017_64\bin\Qt5Gui.dll"
  File "C:\Qt\5.11.1\msvc2017_64\bin\Qt5Widgets.dll"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM Software\MoseleyInstruments\BasicDSP "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BasicDSP" "DisplayName" "BasicDSP"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BasicDSP" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BasicDSP" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BasicDSP" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\BasicDSP"
  CreateShortcut "$SMPROGRAMS\BasicDSP\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  ;createShortCut "$SMPROGRAMS\BasicDSP.lnk" "$INSTDIR\BasicDSP.exe" "" "$INSTDIR\logo.ico"
  createShortCut "$SMPROGRAMS\BasicDSP.lnk" "$INSTDIR\BasicDSP.exe" "" ""
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BasicDSP"
  DeleteRegKey HKLM Software\MoseleyInstruments\BasicDSP

  ; Remove files and uninstaller
  Delete $INSTDIR\uninstall.exe

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\BasicDSP\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\BasicDSP"
  RMDir "$INSTDIR"

SectionEnd
