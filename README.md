# Nonsense Music

Десктопный музыкальный плеер и загрузчик YouTube, написанный на **Qt 6** и **C++20**.

## Возможности

- **Музыкальная библиотека** — импорт локальных аудиофайлов, плейлисты, избранное, просмотр по исполнителям
- **Загрузчик YouTube** — вставь ссылку на YouTube, скачай и конвертируй в MP3 автоматически через [yt-dlp](https://github.com/yt-dlp/yt-dlp) + FFmpeg
- **Воспроизведение** — play, пауза, перемотка, громкость, очередь воспроизведения
- **Мини-плеер** — компактный плавающий плеер для фонового прослушивания
- **Системный трей** — сворачивание в трей, управление воспроизведением из меню трея
- **3 тёмные темы** — Spotify Dark, Coffee Dark (тёплый коричневый), Moss Dark (оливковый)
- **4 языка** — английский, русский, украинский, русский дореформенный
- **Динамический фон** — обложка трека на фоне окна
- **Редактирование метаданных** — изменение информации о треке (название, исполнитель, альбом) через TagLib
- **Облачный аккаунт** — вход/регистрация для облачной синхронизации
- **Обход блокировок YouTube** — ByeDPI прокси, подмена User-Agent под Chrome, использование куки браузера

## Требования

- **Qt 6** (Widgets, Sql, Multimedia, Network)
- **CMake** 3.16+
- **C++20** совместимый компилятор (MinGW или MSVC)
- **yt-dlp.exe** — рядом с исполняемым файлом (автоматически скачивается инсталлятором)
- **ffmpeg.exe** — нужен для конвертации в MP3 (автоматически скачивается инсталлятором)

## Сборка

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

Или открой проект напрямую в **Qt Creator** и собирай оттуда.

## Создание инсталлятора

Требуется [Inno Setup 6](https://jrsoftware.org/isinfo.php). Запусти:

```bash
build_installer.bat
```

Инсталлятор будет создан в `Release_Installer/NonsenseMusic_Setup.exe`. Во время установки он автоматически скачивает последние версии `yt-dlp.exe` и `ffmpeg.exe`.

## Структура проекта

```
src/
  main.cpp                  # Точка входа
  mainwindow.cpp/h          # Главное окно, навигация, трей
  core/
    AudioPlayer.cpp/h       # Воспроизведение аудио (Qt Multimedia)
    LibraryManager.cpp/h    # База SQLite, управление треками и плейлистами
    DownloadManager.cpp/h   # Загрузка YouTube через yt-dlp
    MetadataManager.cpp/h   # Чтение/запись аудио тегов (TagLib)
    CloudManager.cpp/h      # API облачного аккаунта
  ui/
    Sidebar.cpp/h           # Боковая панель навигации
    PlayerBar.cpp/h         # Нижняя панель управления воспроизведением
    LibraryView.cpp/h       # Просмотр треков, плейлистов, исполнителей
    DownloadsView.cpp/h     # Интерфейс загрузки YouTube
    SettingsView.cpp/h      # Настройки приложения (тема, язык, пути)
    AuthView.cpp/h          # Экран входа/регистрации
    MiniPlayer.cpp/h        # Компактный плавающий плеер
    TrackCard.cpp/h         # Карточка трека
    TrackList.cpp/h         # Список треков с сортировкой
    EditTrackDialog.cpp/h   # Диалог редактирования метаданных
    IconProvider.cpp/h      # Пользовательские SVG иконки
  utils/
    TranslationManager.h    # Локализация (EN/RU/UK/RU-PreReform)
    StyleManager.cpp/h      # Определение и переключение тем
resources/
  logo.png, logo.ico        # Иконки приложения
CMakeLists.txt              # Конфигурация сборки
installer.iss               # Скрипт Inno Setup
build_installer.bat         # Автоматизация сборки и упаковки
```

## Лицензия

[Apache-2.0](LICENSE)
