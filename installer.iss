[Setup]
AppName=Nonsense Music
AppVersion=1.0
DefaultDirName={autopf}\NonsenseMusic
DefaultGroupName=Nonsense Music
UninstallDisplayIcon={app}\NonsenseMusic.exe
Compression=lzma2
SolidCompression=yes
OutputDir=Release_Installer
OutputBaseFilename=NonsenseMusic_Setup
PrivilegesRequired=admin

[Files]
Source: "build\NonsenseMusic.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs
Source: "build\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs
Source: "build\sqldrivers\*"; DestDir: "{app}\sqldrivers"; Flags: ignoreversion recursesubdirs
Source: "build\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs
Source: "build\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs
Source: "build\networkinformation\*"; DestDir: "{app}\networkinformation"; Flags: ignoreversion recursesubdirs
Source: "build\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs
Source: "build\multimedia\*"; DestDir: "{app}\multimedia"; Flags: ignoreversion recursesubdirs
Source: "resources\*"; DestDir: "{app}\resources"; Flags: ignoreversion recursesubdirs skipifsourcedoesntexist

[Icons]
Name: "{group}\Nonsense Music"; Filename: "{app}\NonsenseMusic.exe"
Name: "{autodesktop}\Nonsense Music"; Filename: "{app}\NonsenseMusic.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"

[Run]
Filename: "{app}\NonsenseMusic.exe"; Description: "Launch Nonsense Music"; Flags: nowait postinstall skipifsilent

[Code]
var
  DownloadPage: TDownloadWizardPage;

procedure InitializeWizard;
begin
  DownloadPage := CreateDownloadPage(SetupMessage(msgWizardPreparing), SetupMessage(msgPreparingDesc), nil);
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  if CurPageID = wpReady then begin
    DownloadPage.Clear;
    DownloadPage.Add('https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe', 'yt-dlp.exe', '');
    DownloadPage.Add('https://github.com/BtbN/FFmpeg-Builds/releases/download/latest/ffmpeg-master-latest-win64-gpl.zip', 'ffmpeg.zip', '');
    
    DownloadPage.Show;
    try
      try
        DownloadPage.Download;
        Result := True;
      except
        if DownloadPage.AbortedByUser then begin
          Log('Aborted by user.');
        end else begin
          MsgBox('Download failed!', mbError, MB_OK);
        end;
        Result := False;
      end;
    finally
      DownloadPage.Hide;
    end;
  end else begin
    Result := True;
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  ShellApp, ZipFolder, Target: Variant;
  ZipFile, TargetFolder: string;
begin
  if CurStep = ssPostInstall then begin
    // Copy yt-dlp.exe to {app}
    FileCopy(ExpandConstant('{tmp}\yt-dlp.exe'), ExpandConstant('{app}\yt-dlp.exe'), False);
    
    // Extract ffmpeg.zip
    ZipFile := ExpandConstant('{tmp}\ffmpeg.zip');
    TargetFolder := ExpandConstant('{app}\ffmpeg_extracted');
    ForceDirectories(TargetFolder);
    
    try
      ShellApp := CreateOleObject('Shell.Application');
      ZipFolder := ShellApp.NameSpace(ZipFile);
      Target := ShellApp.NameSpace(TargetFolder);
      // 4 = do not display a progress dialog
      // 16 = Respond with "Yes to All" for any dialog box that is displayed
      Target.CopyHere(ZipFolder.Items, 4 or 16);
      
      // Move ffmpeg.exe from subfolder to {app}
      // Usually it's in ffmpeg-master-latest-win64-gpl\bin\ffmpeg.exe
      FileCopy(ExpandConstant('{app}\ffmpeg_extracted\ffmpeg-master-latest-win64-gpl\bin\ffmpeg.exe'), ExpandConstant('{app}\ffmpeg.exe'), False);
      
      // Cleanup
      DelTree(ExpandConstant('{app}\ffmpeg_extracted'), True, True, True);
    except
      MsgBox('Failed to extract FFmpeg automatically. You may need to download it manually.', mbInformation, MB_OK);
    end;
  end;
end;
