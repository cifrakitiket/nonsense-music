#include "LibraryManager.h"
#include <QDir>
#include <QDirIterator>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QDebug>

LibraryManager::LibraryManager(QObject *parent) : QObject(parent) {
    // Determine database and cache directories
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (appData.isEmpty()) {
        appData = QDir::currentPath();
    }
    
    QDir().mkpath(appData);
    m_cacheDir = appData + "/covers/";
    QDir().mkpath(m_cacheDir);
}

LibraryManager::~LibraryManager() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool LibraryManager::initDatabase() {
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (appData.isEmpty()) {
        appData = QDir::currentPath();
    }
    
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(appData + "/nonsense_music.db");
    
    if (!m_db.open()) {
        qCritical() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }
    
    QSqlQuery query;
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS tracks ("
        "file_path TEXT PRIMARY KEY, "
        "title TEXT, "
        "artist TEXT, "
        "album TEXT, "
        "duration INTEGER, "
        "cover_path TEXT, "
        "is_favorite INTEGER DEFAULT 0, "
        "is_cloud INTEGER DEFAULT 0, "
        "cloud_id TEXT"
        ")"
    );
    
    // In case the table already exists from an older version, add the new columns
    query.exec("ALTER TABLE tracks ADD COLUMN is_cloud INTEGER DEFAULT 0");
    query.exec("ALTER TABLE tracks ADD COLUMN cloud_id TEXT");

    if (!success) {
        qCritical() << "Failed to create tracks table:" << query.lastError().text();
        return false;
    }
    
    query.exec(
        "CREATE TABLE IF NOT EXISTS playlists ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT, "
        "cover_path TEXT"
        ")"
    );
    
    query.exec(
        "CREATE TABLE IF NOT EXISTS playlist_tracks ("
        "playlist_id INTEGER, "
        "track_path TEXT, "
        "PRIMARY KEY (playlist_id, track_path)"
        ")"
    );
    
    return true;
}

void LibraryManager::scanDirectory(const QString &dirPath) {
    QDirIterator it(dirPath, QStringList() << "*.mp3" << "*.flac" << "*.wav" << "*.ogg",
                    QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    
    int count = 0;
    while (it.hasNext()) {
        QString path = it.next();
        addTrack(path);
        count++;
        emit scanProgress(path, count);
    }
    
    emit libraryUpdated();
}

void LibraryManager::addTrack(const QString &filePath) {
    QSqlQuery query;
    query.prepare("SELECT file_path FROM tracks WHERE file_path = :path");
    query.bindValue(":path", filePath);
    if (query.exec() && query.next()) {
        // Track already exists in DB
        return;
    }
    
    // Read tags and extract cover
    TrackMetadata meta = MetadataManager::readMetadata(filePath);
    
    QString cachePath;
    if (!meta.coverData.isEmpty()) {
        cacheCover(filePath, meta.coverData);
        cachePath = getCoverCachePath(filePath);
    }
    
    query.prepare("INSERT INTO tracks (file_path, title, artist, album, duration, cover_path) "
                  "VALUES (:path, :title, :artist, :album, :duration, :cover)");
    query.bindValue(":path", filePath);
    query.bindValue(":title", meta.title.isEmpty() ? QFileInfo(filePath).baseName() : meta.title);
    query.bindValue(":artist", meta.artist.isEmpty() ? "Unknown Artist" : meta.artist);
    query.bindValue(":album", meta.album.isEmpty() ? "Unknown Album" : meta.album);
    query.bindValue(":duration", meta.duration);
    query.bindValue(":cover", cachePath);
    
    if (!query.exec()) {
        qWarning() << "Failed to insert track:" << query.lastError().text();
    } else {
        emit trackAddedLocally(meta);
        emit libraryUpdated();
    }
}

void LibraryManager::addCloudTrack(const TrackMetadata &meta) {
    QSqlQuery query;
    query.prepare("SELECT file_path FROM tracks WHERE file_path = :path");
    query.bindValue(":path", meta.filePath);
    if (query.exec() && query.next()) return;
    
    query.prepare("INSERT INTO tracks (file_path, title, artist, album, duration, cover_path, is_cloud, cloud_id) "
                  "VALUES (:path, :title, :artist, :album, :duration, :cover, 1, :cloudId)");
    query.bindValue(":path", meta.filePath);
    query.bindValue(":title", meta.title);
    query.bindValue(":artist", meta.artist);
    query.bindValue(":album", meta.album);
    query.bindValue(":duration", meta.duration);
    query.bindValue(":cover", meta.coverMimeType);
    query.bindValue(":cloudId", meta.cloudId);
    
    if (query.exec()) {
        emit libraryUpdated();
    }
}

void LibraryManager::convertCloudTrackToLocal(const QString &cloudId, const QString &localPath) {
    QSqlQuery query;
    query.prepare("UPDATE tracks SET file_path = :localPath, is_cloud = 0 WHERE cloud_id = :cloudId");
    query.bindValue(":localPath", localPath);
    query.bindValue(":cloudId", cloudId);
    
    if (query.exec()) {
        emit libraryUpdated();
    } else {
        qWarning() << "Failed to convert cloud track to local:" << query.lastError().text();
    }
}

QList<TrackMetadata> LibraryManager::getAllTracks() const {
    QList<TrackMetadata> list;
    QSqlQuery query("SELECT file_path, title, artist, album, duration, cover_path, is_cloud, cloud_id FROM tracks");
    
    while (query.next()) {
        TrackMetadata meta;
        meta.filePath = query.value(0).toString();
        meta.title = query.value(1).toString();
        meta.artist = query.value(2).toString();
        meta.album = query.value(3).toString();
        meta.duration = query.value(4).toInt();
        meta.coverMimeType = query.value(5).toString(); // We temporarily store the cover file path here
        meta.isCloudOnly = query.value(6).toInt() == 1;
        meta.cloudId = query.value(7).toString();
        list.append(meta);
    }
    
    return list;
}

QList<TrackMetadata> LibraryManager::searchTracks(const QString &queryStr) const {
    QList<TrackMetadata> list;
    QSqlQuery query;
    query.prepare("SELECT file_path, title, artist, album, duration, cover_path, is_cloud, cloud_id FROM tracks "
                  "WHERE title LIKE :q OR artist LIKE :q OR album LIKE :q");
    query.bindValue(":q", "%" + queryStr + "%");
    
    if (query.exec()) {
        while (query.next()) {
            TrackMetadata meta;
            meta.filePath = query.value(0).toString();
            meta.title = query.value(1).toString();
            meta.artist = query.value(2).toString();
            meta.album = query.value(3).toString();
            meta.duration = query.value(4).toInt();
            meta.coverMimeType = query.value(5).toString();
            meta.isCloudOnly = query.value(6).toInt() == 1;
            meta.cloudId = query.value(7).toString();
            list.append(meta);
        }
    }
    return list;
}

QList<TrackMetadata> LibraryManager::getFavoriteTracks() const {
    QList<TrackMetadata> list;
    QSqlQuery query("SELECT file_path, title, artist, album, duration, cover_path, is_cloud, cloud_id FROM tracks WHERE is_favorite = 1");
    
    while (query.next()) {
        TrackMetadata meta;
        meta.filePath = query.value(0).toString();
        meta.title = query.value(1).toString();
        meta.artist = query.value(2).toString();
        meta.album = query.value(3).toString();
        meta.duration = query.value(4).toInt();
        meta.coverMimeType = query.value(5).toString();
        meta.isCloudOnly = query.value(6).toInt() == 1;
        meta.cloudId = query.value(7).toString();
        list.append(meta);
    }
    return list;
}

QStringList LibraryManager::getArtists() const {
    QStringList artists;
    QSqlQuery query("SELECT DISTINCT artist FROM tracks WHERE artist != '' ORDER BY artist COLLATE NOCASE");
    while (query.next()) {
        artists.append(query.value(0).toString());
    }
    return artists;
}

QList<TrackMetadata> LibraryManager::getArtistTracks(const QString &artist) const {
    QList<TrackMetadata> tracks;
    QSqlQuery query;
    query.prepare("SELECT filepath, title, artist, album, duration FROM tracks WHERE artist = :artist ORDER BY title COLLATE NOCASE");
    query.bindValue(":artist", artist);
    if (query.exec()) {
        while (query.next()) {
            TrackMetadata m;
            m.filePath = query.value(0).toString();
            m.title    = query.value(1).toString();
            m.artist   = query.value(2).toString();
            m.album    = query.value(3).toString();
            m.duration = query.value(4).toInt();
            tracks.append(m);
        }
    }
    return tracks;
}

bool LibraryManager::updateTrackInfo(const QString &filePath, const QString &title, const QString &artist, const QByteArray &newCover) {
    // 1. Write metadata tags directly on files
    bool success = MetadataManager::writeMetadata(filePath, title, artist, newCover);
    if (!success) {
        return false;
    }
    
    // 2. Cache new cover image if provided
    QString coverPath;
    if (!newCover.isEmpty()) {
        cacheCover(filePath, newCover);
        coverPath = getCoverCachePath(filePath);
    } else {
        // preserve old cover path unless database doesn't have it
        QSqlQuery coverQuery;
        coverQuery.prepare("SELECT cover_path FROM tracks WHERE file_path = :path");
        coverQuery.bindValue(":path", filePath);
        if (coverQuery.exec() && coverQuery.next()) {
            coverPath = coverQuery.value(0).toString();
        }
    }
    
    // 3. Update database record
    QSqlQuery query;
    query.prepare("UPDATE tracks SET title = :title, artist = :artist, cover_path = :cover WHERE file_path = :path");
    query.bindValue(":title", title);
    query.bindValue(":artist", artist);
    query.bindValue(":cover", coverPath);
    query.bindValue(":path", filePath);
    
    if (query.exec()) {
        emit libraryUpdated();
        return true;
    }
    return false;
}

bool LibraryManager::toggleFavorite(const QString &filePath) {
    bool fav = isFavorite(filePath);
    QSqlQuery query;
    query.prepare("UPDATE tracks SET is_favorite = :fav WHERE file_path = :path");
    query.bindValue(":fav", fav ? 0 : 1);
    query.bindValue(":path", filePath);
    
    if (query.exec()) {
        emit libraryUpdated();
        return true;
    }
    return false;
}

bool LibraryManager::isFavorite(const QString &filePath) const {
    QSqlQuery query;
    query.prepare("SELECT is_favorite FROM tracks WHERE file_path = :path");
    query.bindValue(":path", filePath);
    if (query.exec() && query.next()) {
        return query.value(0).toInt() == 1;
    }
    return false;
}

QString LibraryManager::getCoverCachePath(const QString &filePath) const {
    QByteArray hash = QCryptographicHash::hash(filePath.toUtf8(), QCryptographicHash::Md5);
    return m_cacheDir + hash.toHex() + ".jpg";
}

void LibraryManager::cacheCover(const QString &filePath, const QByteArray &coverData) const {
    QString destPath = getCoverCachePath(filePath);
    QFile file(destPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(coverData);
        file.close();
    }
}

bool LibraryManager::deleteTrack(const QString &filePath) {
    QString coverPath = getCoverCachePath(filePath);
    if (QFile::exists(coverPath)) {
        QFile::remove(coverPath);
    }
    
    QSqlQuery query;
    query.prepare("DELETE FROM playlist_tracks WHERE track_path = :path");
    query.bindValue(":path", filePath);
    query.exec();
    
    query.prepare("DELETE FROM tracks WHERE file_path = :path");
    query.bindValue(":path", filePath);
    bool ok = query.exec();
    
    if (ok) {
        emit libraryUpdated();
    }
    return ok;
}

QList<QPair<int, QString>> LibraryManager::getPlaylists() const {
    QList<QPair<int, QString>> list;
    QSqlQuery query("SELECT id, name FROM playlists ORDER BY name ASC");
    while (query.next()) {
        list.append(qMakePair(query.value(0).toInt(), query.value(1).toString()));
    }
    return list;
}

QString LibraryManager::getPlaylistCover(int playlistId) const {
    QSqlQuery query;
    query.prepare("SELECT cover_path FROM playlists WHERE id = :id");
    query.bindValue(":id", playlistId);
    if (query.exec() && query.next()) {
        QString path = query.value(0).toString();
        if (!path.isEmpty() && QFile::exists(path)) {
            return path;
        }
    }
    
    QList<TrackMetadata> tracks = getPlaylistTracks(playlistId);
    for (const auto &track : tracks) {
        QString path = getCoverCachePath(track.filePath);
        if (QFile::exists(path)) {
            return path;
        }
    }
    return "";
}

bool LibraryManager::createPlaylist(const QString &name, const QString &coverPath) {
    QSqlQuery query;
    query.prepare("INSERT INTO playlists (name, cover_path) VALUES (:name, :cover)");
    query.bindValue(":name", name);
    query.bindValue(":cover", coverPath);
    bool ok = query.exec();
    if (ok) {
        emit libraryUpdated();
    }
    return ok;
}

bool LibraryManager::renamePlaylist(int id, const QString &newName) {
    QSqlQuery query;
    query.prepare("UPDATE playlists SET name = :name WHERE id = :id");
    query.bindValue(":name", newName);
    query.bindValue(":id", id);
    bool ok = query.exec();
    if (ok) {
        emit libraryUpdated();
    }
    return ok;
}

bool LibraryManager::updatePlaylistCover(int id, const QString &coverPath) {
    QSqlQuery query;
    query.prepare("UPDATE playlists SET cover_path = :cover WHERE id = :id");
    query.bindValue(":cover", coverPath);
    query.bindValue(":id", id);
    bool ok = query.exec();
    if (ok) {
        emit libraryUpdated();
    }
    return ok;
}

bool LibraryManager::deletePlaylist(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM playlist_tracks WHERE playlist_id = :id");
    query.bindValue(":id", id);
    query.exec();
    
    query.prepare("DELETE FROM playlists WHERE id = :id");
    query.bindValue(":id", id);
    bool ok = query.exec();
    if (ok) {
        emit libraryUpdated();
    }
    return ok;
}

bool LibraryManager::addTrackToPlaylist(int playlistId, const QString &trackPath) {
    if (isTrackInPlaylist(playlistId, trackPath)) {
        return true;
    }
    QSqlQuery query;
    query.prepare("INSERT INTO playlist_tracks (playlist_id, track_path) VALUES (:id, :path)");
    query.bindValue(":id", playlistId);
    query.bindValue(":path", trackPath);
    bool ok = query.exec();
    if (ok) {
        emit libraryUpdated();
    }
    return ok;
}

bool LibraryManager::removeTrackFromPlaylist(int playlistId, const QString &trackPath) {
    QSqlQuery query;
    query.prepare("DELETE FROM playlist_tracks WHERE playlist_id = :id AND track_path = :path");
    query.bindValue(":id", playlistId);
    query.bindValue(":path", trackPath);
    bool ok = query.exec();
    if (ok) {
        emit libraryUpdated();
    }
    return ok;
}

QList<TrackMetadata> LibraryManager::getPlaylistTracks(int playlistId) const {
    QList<TrackMetadata> list;
    QSqlQuery query;
    query.prepare("SELECT file_path, title, artist, album, duration, cover_path "
                  "FROM tracks t "
                  "JOIN playlist_tracks pt ON t.file_path = pt.track_path "
                  "WHERE pt.playlist_id = :id");
    query.bindValue(":id", playlistId);
    
    if (query.exec()) {
        while (query.next()) {
            TrackMetadata meta;
            meta.filePath = query.value(0).toString();
            meta.title = query.value(1).toString();
            meta.artist = query.value(2).toString();
            meta.album = query.value(3).toString();
            meta.duration = query.value(4).toInt();
            meta.coverMimeType = query.value(5).toString();
            list.append(meta);
        }
    }
    return list;
}

bool LibraryManager::isTrackInPlaylist(int playlistId, const QString &trackPath) const {
    QSqlQuery query;
    query.prepare("SELECT 1 FROM playlist_tracks WHERE playlist_id = :id AND track_path = :path");
    query.bindValue(":id", playlistId);
    query.bindValue(":path", trackPath);
    if (query.exec() && query.next()) {
        return true;
    }
    return false;
}
