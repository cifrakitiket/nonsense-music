#ifndef METADATAMANAGER_H
#define METADATAMANAGER_H

#include <QString>
#include <QByteArray>

struct TrackMetadata {
    QString filePath;
    QString title;
    QString artist;
    QString album;
    int duration = 0; // in seconds
    QByteArray coverData;
    QString coverMimeType;
    
    bool isCloudOnly = false;
    QString cloudId = "";
};

class MetadataManager {
public:
    static TrackMetadata readMetadata(const QString &filePath);
    static bool writeMetadata(const QString &filePath, const QString &title, const QString &artist, const QByteArray &coverData = QByteArray());
};

#endif // METADATAMANAGER_H
