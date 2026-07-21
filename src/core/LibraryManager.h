#ifndef LIBRARYMANAGER_H
#define LIBRARYMANAGER_H

#include <QObject>
#include <QStringList>
#include <QSqlDatabase>
#include "MetadataManager.h"

class LibraryManager : public QObject {
    Q_OBJECT
public:
    explicit LibraryManager(QObject *parent = nullptr);
    ~LibraryManager();

    bool initDatabase();
    void scanDirectory(const QString &dirPath);
    void addTrack(const QString &filePath);
    void addCloudTrack(const TrackMetadata &meta);
    void convertCloudTrackToLocal(const QString &cloudId, const QString &localPath);
    bool deleteTrack(const QString &filePath);
    
    QList<TrackMetadata> getAllTracks() const;
    QList<TrackMetadata> searchTracks(const QString &query) const;
    QList<TrackMetadata> getFavoriteTracks() const;
    
    QStringList getArtists() const;
    QList<TrackMetadata> getArtistTracks(const QString &artist) const;
    
    bool updateTrackInfo(const QString &filePath, const QString &title, const QString &artist, const QByteArray &newCover = QByteArray());
    bool toggleFavorite(const QString &filePath);
    bool isFavorite(const QString &filePath) const;
    
    // Playlists CRUD
    QList<QPair<int, QString>> getPlaylists() const;
    QString getPlaylistCover(int playlistId) const;
    bool createPlaylist(const QString &name, const QString &coverPath = "");
    bool renamePlaylist(int id, const QString &newName);
    bool updatePlaylistCover(int id, const QString &coverPath);
    bool deletePlaylist(int id);
    bool addTrackToPlaylist(int playlistId, const QString &trackPath);
    bool removeTrackFromPlaylist(int playlistId, const QString &trackPath);
    QList<TrackMetadata> getPlaylistTracks(int playlistId) const;
    bool isTrackInPlaylist(int playlistId, const QString &trackPath) const;
    
    QString getCoverCachePath(const QString &filePath) const;
    
signals:
    void libraryUpdated();
    void scanProgress(const QString &currentFile, int count);
    void trackAddedLocally(const TrackMetadata &meta);
    
private:
    QSqlDatabase m_db;
    QString m_cacheDir;
    
    void cacheCover(const QString &filePath, const QByteArray &coverData) const;
};

#endif // LIBRARYMANAGER_H
