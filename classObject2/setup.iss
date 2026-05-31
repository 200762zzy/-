; Inno Setup 脚本 — 实验报告AI自动评语生成工具
; 使用前请先运行 windeployqt classObject2.exe 收集依赖

#define MyAppName "实验报告AI自动评语生成工具"
#define MyAppVersion "1.0"
#define MyAppPublisher "ClassObject2"
#define MyAppExeName "classObject2.exe"

[Setup]
AppId={{B8F4C3A2-1D5E-4A7F-9C6B-0E3D2F1A8B4C}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputDir=.
OutputBaseFilename=classObject2_Setup
Compression=lzma
SolidCompression=yes
UninstallDisplayIcon={app}\{#MyAppExeName}
PrivilegesRequired=admin

[Languages]
Name: "chinesesimplified"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"

[Tasks]
Name: "desktopicon"; Description: "创建桌面快捷方式"; GroupDescription: "附加任务:"

[Files]
Source: "release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "release\*.dll"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs
Source: "release\*.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "release\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs
Source: "release\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs
Source: "release\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs
Source: "release\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs
Source: "release\bearer\*"; DestDir: "{app}\bearer"; Flags: ignoreversion recursesubdirs
Source: "release\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs
Source: "release\mediaservice\*"; DestDir: "{app}\mediaservice"; Flags: ignoreversion recursesubdirs
Source: "release\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs
Source: "release\printsupport\*"; DestDir: "{app}\printsupport"; Flags: ignoreversion recursesubdirs
Source: "release\sqldrivers\*"; DestDir: "{app}\sqldrivers"; Flags: ignoreversion recursesubdirs
Source: "release\Qt5*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "release\lib*.dll"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\卸载 {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "立即运行 {#MyAppName}"; Flags: postinstall nowait skipifsilent
