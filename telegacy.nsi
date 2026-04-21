!include "MUI2.nsh"

!define APPNAME "Telegacy"
!define APPVER "1.0.1"

Name "${APPNAME}"
OutFile "${APPNAME}-${APPVER}-Setup.exe"
InstallDir "$PROGRAMFILES\${APPNAME}"

RequestExecutionLevel admin
SetCompressor /SOLID lzma
ShowInstDetails show
ShowUninstDetails show

!define MUI_ICON "telegacy.ico"
!define MUI_UNICON "uninstall.ico"

VIProductVersion "1.0.1.0"
VIAddVersionKey "ProductName" "${APPNAME}"
VIAddVersionKey "FileVersion" "${APPVER}"
VIAddVersionKey "FileDescription" "${APPNAME} Setup"
VIAddVersionKey "LegalCopyright" "Copyright © 2026 N3xtery"
VIAddVersionKey "OriginalFilename" "${APPNAME}-${APPVER}-Setup.exe"

!define MUI_ABORTWARNING

!define MUI_FINISHPAGE_RUN "$INSTDIR\telegacy.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch Telegacy"
!define MUI_COMPONENTSPAGE_NODESC
!define MUI_LICENSEPAGE_BUTTON "Next >"
!define MUI_LICENSEPAGE_TEXT_BOTTOM "Telegacy is released under the GNU General Public License (GPL). The license is provided here for information purposes only. Click Next to continue."

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "gpl-3.0.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES


Section "Telegacy" SecMain
  SectionIn RO 

  SetShellVarContext all
  SetOutPath "$INSTDIR"
  File "Release\telegacy.exe"

  SetOverwrite ifnewer
  File "Debug\help.hlp"
  File "Debug\help.cnt"
  File "Debug\help.chm"
  File "Debug\emoji_categories.dat"

  SetOutPath "$INSTDIR\emojis"
  File /r /x "Thumbs.db" "Debug\emojis\*.*"

  Call IsWin9x
  Pop $0
  StrCmp $0 1 0 +3
    SetOutPath "$SYSDIR"
    SetOverwrite ifnewer
    File "dlls\unicows.dll"

  SetOutPath "$SYSDIR"
  SetOverwrite ifnewer
  File "dlls\riched20.dll"
  File "dlls\msls31.dll"
  SetOverwrite off
  File "dlls\msvcrt.dll"


WriteUninstaller "$INSTDIR\uninstall.exe"

WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Telegacy" "DisplayName" "${APPNAME} ${APPVER}"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Telegacy" "DisplayIcon" "$INSTDIR\telegacy.exe"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Telegacy" "UninstallString" "$INSTDIR\uninstall.exe"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Telegacy" "QuietUninstallString" "$INSTDIR\uninstall.exe /S"
WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Telegacy" "NoModify" 1
WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Telegacy" "NoRepair" 1
SectionEnd

Section "Create a Start Menu folder" SecStartMenu
  Call IsNT3
  Pop $0
  StrCmp $0 1 +5
  CreateDirectory "$SMPROGRAMS\Telegacy"
  CreateShortCut "$SMPROGRAMS\Telegacy\Telegacy.lnk" "$INSTDIR\telegacy.exe"
  CreateShortCut "$SMPROGRAMS\Telegacy\Uninstall Telegacy.lnk" "$INSTDIR\uninstall.exe"
  Goto skip
  ExecWait '"$INSTDIR\telegacy.exe" /progman_install'
  skip:
SectionEnd

Section /o "Create a desktop icon" SecDesktop
  CreateShortCut "$DESKTOP\Telegacy.lnk" "$INSTDIR\telegacy.exe"
SectionEnd

Section /o "Create a Quick Launch icon" SecQuickLaunch
  CreateShortCut "$QUICKLAUNCH\Telegacy.lnk" "$INSTDIR\telegacy.exe"
SectionEnd

Section "Uninstall"
  SetShellVarContext all
  ExecWait '"$INSTDIR\telegacy.exe" /uninstall'

  Delete "$DESKTOP\Telegacy.lnk"
  Delete "$QUICKLAUNCH\Telegacy.lnk"
  Call un.IsNT3
  Pop $0
  StrCmp $0 1 +5
  Delete "$SMPROGRAMS\Telegacy\Telegacy.lnk"
  Delete "$SMPROGRAMS\Telegacy\Uninstall Telegacy.lnk"
  RMDIR "$SMPROGRAMS\Telegacy"
  Goto unskip
  ExecWait '"$INSTDIR\telegacy.exe" /progman_uninstall'
  unskip:

  Delete "$INSTDIR\telegacy.exe"
  Delete "$INSTDIR\help.hlp"
  Delete "$INSTDIR\help.cnt"
  Delete "$INSTDIR\help.chm"
  Delete "$INSTDIR\emoji_categories.dat"
  RMDIR /r "$INSTDIR\emojis"
  Delete "$INSTDIR\uninstall.exe"
  RMDir "$INSTDIR"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Telegacy"
SectionEnd

Function .onInit
  Call IsNT3
  Pop $0
  StrCmp $0 0 +4
    SectionSetText ${SecStartMenu} "Create a Program Manager group"
    SectionSetText ${SecDesktop} ""
    SectionSetText ${SecQuickLaunch} ""
FunctionEnd

Function IsWin9x
  System::Call 'kernel32::GetVersion() i .r0'
  IntOp $0 $0 & 0x80000000
  StrCmp $0 0 not9x
  StrCpy $0 1
  Goto done
  not9x:
    StrCpy $0 0
  done:
  Push $0
FunctionEnd

!macro IsNT3Macro
  System::Call 'kernel32::GetVersion() i .r0'
  IntOp $2 $0 & 0xFF
  StrCmp $2 3 nt3x
  StrCpy $0 0
  Goto done
  nt3x:
    StrCpy $0 1
  done:
  Push $0
!macroend

Function IsNT3
  !insertmacro IsNT3Macro
FunctionEnd

Function un.IsNT3
  !insertmacro IsNT3Macro
FunctionEnd