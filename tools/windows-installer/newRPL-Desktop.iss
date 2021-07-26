[Setup]
AppName=newRPL Desktop
AppPublisher=The newRPL Team
AppVersion=Build 1480
WizardStyle=modern
DefaultDirName={autopf}\newRPL Desktop
DefaultGroupName=newRPL Desktop
UninstallDisplayIcon={app}\newRPL Desktop_50g.exe
OutputDir=userdocs:deploy-newRPL-Desktop\Installer-Bin
OutputBaseFilename=newRPL Desktop Installer
SourceDir=userdocs:deploy-newRPL-Desktop
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog

[Types]
Name: "full"; Description: "Full installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "qtlibs"; Description: "Qt Runtime DLLs"; Types: full custom; Flags: fixed
Name: "gcclibs"; Description: "Gcc needed DLL libraries"; Types: full custom; Flags: fixed
Name: "primeinstaller"; Description: "Initial firmware installer for Prime G1 target"; Types: full
Name: "desktop50g"; Description: "newRPL Desktop - 50g look"; Types: full
Name: "desktopprime"; Description: "newRPL Desktop - Prime look"; Types: full

[Files]
Source: "Qt5Core.dll"; DestDir: "{app}"; Components: qtlibs
Source: "Qt5Gui.dll"; DestDir: "{app}"; Components: qtlibs
Source: "Qt5Widgets.dll"; DestDir: "{app}"; Components: qtlibs

Source: "libgcc_s_dw2-1.dll"; DestDir: "{app}"; Components: gcclibs
Source: "libstdc++-6.dll"; DestDir: "{app}"; Components: gcclibs
Source: "libwinpthread-1.dll"; DestDir: "{app}"; Components: gcclibs

Source: "newRPL Desktop-50g.exe"; DestDir: "{app}"; Components: desktop50g
Source: "newRPL Desktop-Prime.exe"; DestDir: "{app}"; Components: desktopprime
Source: "newRPL-Prime Firmware Bundler.exe"; DestDir: "{app}"; Components: primeinstaller

[Icons]
Name: "{group}\newRPL Desktop (50g look)"; Filename: "{app}\newRPL Desktop-50g.exe"; WorkingDir: "{userdocs}/newRPL Files"; Components: desktop50g;
Name: "{group}\newRPL Desktop (Prime look)"; Filename: "{app}\newRPL Desktop-Prime.exe"; WorkingDir: "{userdocs}/newRPL Files"; Components: desktopprime;
Name: "{group}\newRPL Prime Firmware Bundler"; Filename: "{app}\newRPL-Prime Firmware Bundler.exe"; Components: primeinstaller
Name: "{group}\Uninstall newRPL"; Filename: "{uninstallexe}"
