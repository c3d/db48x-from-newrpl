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
LicenseFile=license\LICENSE.txt

[Types]
Name: "full"; Description: "Full installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "qtlibs"; Description: "Qt Runtime DLLs"; Types: full custom; Flags: fixed
Name: "gcclibs"; Description: "Gcc needed DLL libraries"; Types: full custom; Flags: fixed
Name: "primeinstaller"; Description: "Initial firmware installer for Prime G1 target"; Types: full
Name: "desktop50g"; Description: "newRPL Desktop - 50g look"; Types: full
Name: "desktopprime"; Description: "newRPL Desktop - Prime look"; Types: full
Name: "desktopprime\samplethemes"; Description: "Sample color themes"; Types: full

[Dirs]
Name: "{autodocs}\newRPL Files"; Components: desktop50g desktopprime
Name: "{autodocs}\newRPL Files\Sample Themes"; Components: desktopprime\samplethemes



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

Source: "sample themes\Color_Theme_AmberCRT.nrpl"; DestDir: "{autodocs}\newRPL Files\Sample Themes"; Components: "desktopprime\samplethemes"
Source: "sample themes\Color_Theme_Default.nrpl"; DestDir: "{autodocs}\newRPL Files\Sample Themes"; Components: "desktopprime\samplethemes"
Source: "sample themes\Color_Theme_GreenCRT.nrpl"; DestDir: "{autodocs}\newRPL Files\Sample Themes"; Components: "desktopprime\samplethemes"
Source: "sample themes\Color_Theme_Red.nrpl"; DestDir: "{autodocs}\newRPL Files\Sample Themes"; Components: "desktopprime\samplethemes"
Source: "license\LICENSE.txt"; DestDir: "{app}"; Components: desktopprime desktop50g primeinstaller




[Icons]
Name: "{group}\newRPL Desktop (50g look)"; Filename: "{app}\newRPL Desktop-50g.exe"; WorkingDir: "{userdocs}\newRPL Files"; Components: desktop50g;
Name: "{group}\newRPL Desktop (Prime look)"; Filename: "{app}\newRPL Desktop-Prime.exe"; WorkingDir: "{userdocs}\newRPL Files"; Components: desktopprime;
Name: "{group}\newRPL Prime Firmware Bundler"; Filename: "{app}\newRPL-Prime Firmware Bundler.exe"; Components: primeinstaller
Name: "{group}\Uninstall newRPL"; Filename: "{uninstallexe}"
