#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QString>
#include <QMap>
#include <QSettings>

class TranslationManager {
public:
    enum class Language {
        English = 0,
        Russian = 1,
        Ukrainian = 2,
        RussianPreReform = 3
    };

    static TranslationManager& instance() {
        static TranslationManager inst;
        return inst;
    }

    void setLanguage(Language lang) {
        m_currentLang = lang;
        QSettings settings("NonsenseMusic", "Player");
        settings.setValue("language", static_cast<int>(lang));
    }

    Language currentLanguage() const {
        return m_currentLang;
    }

    QString get(const QString &key) const {
        if (m_translations.contains(key)) {
            return m_translations[key][m_currentLang];
        }
        return key;
    }

private:
    Language m_currentLang;
    QMap<QString, QMap<Language, QString>> m_translations;

    TranslationManager() {
        // Load saved language
        QSettings settings("NonsenseMusic", "Player");
        m_currentLang = static_cast<Language>(settings.value("language", static_cast<int>(Language::Russian)).toInt());

        auto add = [this](const QString &key, const QString &en, const QString &ru, const QString &uk, const QString &ruPre) {
            QMap<Language, QString> map;
            map[Language::English] = en;
            map[Language::Russian] = ru;
            map[Language::Ukrainian] = uk;
            map[Language::RussianPreReform] = ruPre;
            m_translations[key] = map;
        };

        // Sidebar Navigation
        add("sidebar_library", "Library", "Библиотека", "Бібліотека", "Библіотека");
        add("sidebar_downloads", "Downloads", "Загрузки", "Завантаження", "Загрузки");
        add("sidebar_account", "Account", "Аккаунт", "Акаунт", "Аккаунтъ");
        add("sidebar_settings", "Settings", "Настройки", "Налаштування", "Настройки");
        add("sidebar_your_library", "Your Library", "Моя медиатека", "Моя медіатека", "Моя медіатека");
        add("sidebar_artists", "Artists", "Исполнители", "Виконавці", "Исполнители");
        add("sidebar_recents", "Recents", "Недавние", "Недавні", "Недавніе");

        // Tab Selector inside Library
        add("tracks_tab", "Tracks", "Треки", "Треки", "Треки");
        add("playlists_tab", "Playlists", "Плейлисты", "Плейлисти", "Плейлисты");
        add("search_placeholder", "🔍 Search tracks, artists, albums...", "🔍 Поиск треков, артистов, альбомов...", "🔍 Пошук треків, артистів, альбомів...", "🔍 Поискъ трековъ, артистовъ, альбомовъ...");
        add("add_files", "➕ Add File(s)...", "➕ Добавить файлы...", "➕ Додати файли...", "➕ Добавить файлы...");

        // Downloads Screen
        add("dl_title", "Downloads", "Загрузки", "Завантаження", "Загрузки");
        add("dl_subtitle", "Paste a YouTube URL below to download and convert music to MP3 format.", "Вставьте ссылку на YouTube ниже, чтобы скачать и сконвертировать в MP3.", "Вставте посилання на YouTube нижче, щоб завантажити та конвертувати в MP3.", "Вставьте ссылку на YouTube ниже, чтобы скачать и сконвертировать въ MP3.");
        add("btn_download", "Download", "Скачать", "Завантажити", "Скачать");
        add("btn_cancel", "Cancel", "Отмена", "Скасувати", "Отмѣна");
        add("bypass_dpi", "Bypass YouTube blocking (via ByeDPI)", "Обход блокировки YouTube (через ByeDPI)", "Обхід блокування YouTube (через ByeDPI)", "Обходъ блокировки YouTube (черезъ ByeDPI)");
        add("mimic_chrome", "Mimic Google Chrome browser client", "Маскировка под браузер Google Chrome", "Маскування під браузер Google Chrome", "Маскировка подъ браузеръ Google Chrome");
        add("use_cookies", "Use Chrome browser cookies (bypass bot check)", "Использовать куки браузера Chrome", "Використовувати куки браузера Chrome", "Использовать куки браузера Chrome");

        // Download Bypass Section
        add("dl_bypass_header", "Bypass & Anti-Blocking", "Обход блокировок", "Обхід блокувань", "Обходъ блокировокъ");
        add("dl_use_cookies", "Use Chrome cookies (bypass bot check)", "Куки Chrome (обход бот-проверки)", "Куки Chrome (обхід бот-перевірки)", "Куки Chrome (обходъ бот-проверки)");
        add("dl_player_client", "Player client:", "Клиент плеера:", "Клієнт плеєра:", "Клієнт плеєра:");
        add("dl_pc_default", "Default (auto)", "По умолчанию (авто)", "За замовчуванням (авто)", "По умолчанию (авто)");
        add("dl_use_byedpi", "ByeDPI proxy (DPI bypass):", "ByeDPI прокси (обход DPI):", "ByeDPI проксі (обхід DPI):", "ByeDPI прокси (обходъ DPI):");
        add("dl_geo_bypass", "Geo-bypass (region unlock):", "Гео-обход (разблокировка региона):", "Гео-обхід (розблокування регіону):", "Гео-обходъ (разблокировкаъ регіона):");
        add("dl_geo_country_hint", "Country code (US, DE...)", "Код страны (US, DE...)", "Код країни (US, DE...)", "Кодъ страны (US, DE...)");
        add("dl_legacy_ssl", "Legacy SSL", "Старый SSL", "Старий SSL", "Старый SSL");
        add("dl_proxy", "Proxy:", "Прокси:", "Проксі:", "Прокси:");

        // Settings Screen
        add("settings_title", "Settings", "Настройки", "Налаштування", "Настройки");
        add("settings_dynamic_bg", "Dynamic Background (Cover Art)", "Динамический фон (обложка трека)", "Динамічний фон (обкладинка треку)", "Динамическій фонъ (обложка трека)");
        add("settings_dl_header", "Downloader Configuration", "Настройки загрузчика", "Налаштування завантажувача", "Настройки загрузчика");
        add("settings_dl_path", "Path to yt-dlp.exe", "Путь к yt-dlp.exe", "Шлях до yt-dlp.exe", "Путь къ yt-dlp.exe");
        add("settings_dl_browse", "Browse...", "Обзор...", "Огляд...", "Обзоръ...");
        add("settings_dl_dir_header", "YouTube Download Location", "Папка для загрузок с YouTube", "Папка для завантажень з YouTube", "Папка для загрузокъ съ YouTube");
        add("settings_dl_dir_placeholder", "Default: Music/NonsenseMusic", "По умолчанию: Музыка/NonsenseMusic", "За замовчуванням: Музика/NonsenseMusic", "По умолчанию: Музыка/NonsenseMusic");
        add("settings_lang_header", "Application Language", "Язык приложения", "Мова програми", "Языкъ приложенія");

        // General UI / Actions
        add("favorites_only", "Favorites Only", "Только Избранное", "Тільки Обране", "Только Избранное");
        add("favorites_playlist", "Favorites", "Избранное", "Обране", "Избранное");
        add("edit_info", "Edit Info...", "Изменить инфо...", "Редагувати інфо...", "Измѣнить инфо...");
        add("delete_track", "Delete Track", "Удалить трек", "Видалити трек", "Удалить трекъ");
        
        // Deletion confirmation
        add("delete_confirm_title", "Delete Track", "Удаление трека", "Видалення треку", "Удаленіе трека");
        add("delete_confirm_text", "Are you sure you want to remove this track from your library?", "Вы уверены, что хотите удалить этот трек из библиотеки?", "Ви впевнені, що хочете видалити цей трек з медіатеки?", "Вы уверены, что хотите удалить этотъ трекъ изъ библіотеки?");
        
        // Playlists Menu & Dialog Actions
        add("playlist_new", "🆕 Create Playlist", "🆕 Создать плейлист", "🆕 Створити плейлист", "🆕 Создать плейлистъ");
        add("playlist_rename", "✏ Rename Playlist", "✏ Переименовать плейлист", "✏ Перейменувати плейлист", "✏ Переименовать плейлистъ");
        add("playlist_change_cover", "🖼 Change Cover", "🖼 Изменить обложку", "🖼 Змінити обкладинку", "🖼 Измѣнить обложку");
        add("playlist_delete", "❌ Delete Playlist", "❌ Удалить плейлист", "❌ Видалити плейлист", "❌ Удалить плейлистъ");
        add("add_to_playlist", "➕ Add to Playlist", "➕ Добавить в плейлист", "➕ Додати до плейлиста", "➕ Добавить въ плейлистъ");
        add("remove_from_playlist", "➖ Remove from Playlist", "➖ Удалить из плейлиста", "➖ Видалити з плейлиста", "➖ Удалить изъ плейлиста");
        add("playlist_back", "⬅ Back to Playlists", "⬅ Назад к плейлистам", "⬅ Назад до плейлистів", "⬅ Назадъ къ плейлистамъ");
        add("playlist_name_prompt", "Enter Playlist Name:", "Введите название плейлиста:", "Введіть назву плейлиста:", "Введите названіе плейлиста:");

        // Theme
        add("theme_change_title", "Theme Changed", "Смена темы", "Зміна теми", "Смѣна темы");
        add("theme_change_restart_message", "Please restart the application for the theme to apply completely.", "Пожалуйста, перезапустите приложение, чтобы тема применилась полностью.", "Будь ласка, перезапустіть програму, щоб тема застосувалася повністю.", "Пожалуйста, перезапустите приложеніе, чтобы тема примѣнилась полностью.");
    }
};

#define trL(key) TranslationManager::instance().get(key)

#endif // TRANSLATIONMANAGER_H
