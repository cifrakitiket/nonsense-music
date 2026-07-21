# Nonsense Music

Desktop music player and YouTube downloader built with **Qt 6** and **C++20**.

## Features

- **Music Library** — import local audio files, organize into playlists, mark favorites, browse by artists
- **YouTube Downloader** — paste a YouTube link, download and convert to MP3 automatically via [yt-dlp](https://github.com/yt-dlp/yt-dlp) + FFmpeg
- **Audio Playback** — play, pause, seek, volume control, playback queue
- **Mini Player** — compact floating player for background listening
- **System Tray** — minimize to tray, control playback from tray menu
- **3 Dark Themes** — Spotify Dark, Coffee Dark (warm brown), Moss Dark (olive green)
- **4 Languages** — English, Russian, Ukrainian, Russian Pre-Reform
- **Dynamic Background** — album art as window background
- **Metadata Editing** — edit track info (title, artist, album) via TagLib
- **Cloud Account** — login/register for cloud sync
- **YouTube Bypass Options** — ByeDPI proxy, Chrome user-agent spoofing, browser cookies support

## Requirements

- **Qt 6** (Widgets, Sql, Multimedia, Network)
- **CMake** 3.16+
- **C++20** compatible compiler (MinGW or MSVC)
- **yt-dlp.exe** — placed next to the built executable (auto-downloaded by installer)
- **ffmpeg.exe** — required for MP3 conversion (auto-downloaded by installer)

## Building

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

Or open the project directly in **Qt Creator** and build from there.

## Creating an Installer

Requires [Inno Setup 6](https://jrsoftware.org/isinfo.php). Run:

```bash
build_installer.bat
```

The installer will be created at `Release_Installer/NonsenseMusic_Setup.exe`. It automatically downloads the latest `yt-dlp.exe` and `ffmpeg.exe` during installation.

## Project Structure

```
src/
  main.cpp                  # Entry point
  mainwindow.cpp/h          # Main window, tab navigation, tray
  core/
    AudioPlayer.cpp/h       # Audio playback (Qt Multimedia)
    LibraryManager.cpp/h    # SQLite database, track/playlist management
    DownloadManager.cpp/h   # YouTube download via yt-dlp
    MetadataManager.cpp/h   # Audio tag read/write (TagLib)
    CloudManager.cpp/h      # Cloud account API
  ui/
    Sidebar.cpp/h           # Left navigation panel
    PlayerBar.cpp/h         # Bottom player controls
    LibraryView.cpp/h       # Tracks, playlists, artists browser
    DownloadsView.cpp/h     # YouTube download interface
    SettingsView.cpp/h      # App settings (theme, language, paths)
    AuthView.cpp/h          # Login/register screen
    MiniPlayer.cpp/h        # Compact floating player
    TrackCard.cpp/h         # Track display card
    TrackList.cpp/h         # Track list with sorting
    EditTrackDialog.cpp/h   # Edit track metadata dialog
    IconProvider.cpp/h      # Custom SVG icons
  utils/
    TranslationManager.h    # i18n (EN/RU/UK/RU-PreReform)
    StyleManager.cpp/h      # Theme definitions and switching
resources/
  logo.png, logo.ico        # Application icons
CMakeLists.txt              # Build configuration
installer.iss               # Inno Setup script
build_installer.bat         # Build + package automation
```

## License

[Apache-2.0](LICENSE)
