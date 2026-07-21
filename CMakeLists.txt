cmake_minimum_required(VERSION 3.16)
project(NonsenseMusic VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 6 Configuration
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Multimedia Network)

# Enable automatic compilation tools for Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Fetch TagLib from GitHub
include(FetchContent)
FetchContent_Declare(
    taglib
    GIT_REPOSITORY https://github.com/taglib/taglib.git
    GIT_TAG v2.0.2
)

# Configure TagLib build options
set(BUILD_TESTS OFF CACHE BOOL "Build TagLib tests" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Build TagLib examples" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build TagLib as a shared library" FORCE)
set(WITH_ZLIB OFF CACHE BOOL "Build TagLib with zlib support" FORCE)

FetchContent_MakeAvailable(taglib)

# Headers and Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    
    src/core/AudioPlayer.h
    src/core/AudioPlayer.cpp
    src/core/LibraryManager.h
    src/core/LibraryManager.cpp
    src/core/MetadataManager.h
    src/core/MetadataManager.cpp
    src/core/DownloadManager.h
    src/core/DownloadManager.cpp
    src/core/CloudManager.h
    src/core/CloudManager.cpp
    
    src/ui/Sidebar.h
    src/ui/Sidebar.cpp
    src/ui/PlayerBar.h
    src/ui/PlayerBar.cpp
    src/ui/TrackCard.h
    src/ui/TrackCard.cpp
    src/ui/TrackList.h
    src/ui/TrackList.cpp
    src/ui/DownloadsView.h
    src/ui/DownloadsView.cpp
    src/ui/SettingsView.h
    src/ui/SettingsView.cpp
    src/ui/EditTrackDialog.h
    src/ui/EditTrackDialog.cpp
    src/ui/LibraryView.h
    src/ui/LibraryView.cpp
    src/ui/MiniPlayer.h
    src/ui/MiniPlayer.cpp
    src/ui/AuthView.h
    src/ui/AuthView.cpp
    src/utils/TranslationManager.h
    src/utils/StyleManager.h
    src/utils/StyleManager.cpp
    src/ui/IconProvider.h
    src/ui/IconProvider.cpp
)

# App executable
# resources.qrc — embeds logo.png/logo.ico as :/logo.png etc. (Qt resource system)
# app.rc — sets the .exe icon shown in Windows Explorer (MinGW windres)
if(WIN32)
    set(APP_ICON_RC src/app.rc)
else()
    set(APP_ICON_RC "")
endif()

add_executable(NonsenseMusic ${SOURCES} src/resources.qrc ${APP_ICON_RC})

# Target Compile Definitions
target_compile_definitions(NonsenseMusic PRIVATE TAGLIB_STATIC)

# Target Include Directories
target_include_directories(NonsenseMusic PRIVATE
    src
    src/core
    src/ui
    src/utils
    ${taglib_SOURCE_DIR}
    ${taglib_BINARY_DIR}
    ${taglib_SOURCE_DIR}/taglib
    ${taglib_BINARY_DIR}/taglib
    ${taglib_SOURCE_DIR}/taglib/toolkit
    ${taglib_SOURCE_DIR}/taglib/asf
    ${taglib_SOURCE_DIR}/taglib/mpeg
    ${taglib_SOURCE_DIR}/taglib/ogg
    ${taglib_SOURCE_DIR}/taglib/ogg/flac
    ${taglib_SOURCE_DIR}/taglib/flac
    ${taglib_SOURCE_DIR}/taglib/mpc
    ${taglib_SOURCE_DIR}/taglib/mp4
    ${taglib_SOURCE_DIR}/taglib/ogg/vorbis
    ${taglib_SOURCE_DIR}/taglib/ogg/speex
    ${taglib_SOURCE_DIR}/taglib/ogg/opus
    ${taglib_SOURCE_DIR}/taglib/mpeg/id3v2
    ${taglib_SOURCE_DIR}/taglib/mpeg/id3v2/frames
    ${taglib_SOURCE_DIR}/taglib/mpeg/id3v1
    ${taglib_SOURCE_DIR}/taglib/ape
    ${taglib_SOURCE_DIR}/taglib/wavpack
    ${taglib_SOURCE_DIR}/taglib/trueaudio
    ${taglib_SOURCE_DIR}/taglib/riff
    ${taglib_SOURCE_DIR}/taglib/riff/aiff
    ${taglib_SOURCE_DIR}/taglib/riff/wav
    ${taglib_SOURCE_DIR}/taglib/mod
    ${taglib_SOURCE_DIR}/taglib/s3m
    ${taglib_SOURCE_DIR}/taglib/it
    ${taglib_SOURCE_DIR}/taglib/xm
)

# Link Libraries
# TagLib exports target tag::tag
target_link_libraries(NonsenseMusic PRIVATE
    Qt6::Widgets
    Qt6::Sql
    Qt6::Multimedia
    Qt6::Network
    tag
)

# Optimization/Platform settings for Windows
if(WIN32)
    # Hide console window in Release builds
    set_target_properties(NonsenseMusic PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Copy yt-dlp.exe to the output directory
add_custom_command(TARGET NonsenseMusic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/yt-dlp.exe"
    $<TARGET_FILE_DIR:NonsenseMusic>
)

# Run windeployqt automatically on Windows
if(WIN32)
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
    
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET NonsenseMusic POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}" --release --no-translations --no-opengl-sw --no-compiler-runtime "$<TARGET_FILE:NonsenseMusic>"
            COMMENT "Deploying Qt dependencies via windeployqt..."
        )
    endif()
endif()
