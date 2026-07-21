#ifndef CLOUDMANAGER_H
#define CLOUDMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include "MetadataManager.h"

class CloudManager : public QObject {
    Q_OBJECT
public:
    explicit CloudManager(QObject *parent = nullptr);
    ~CloudManager();

    // Setup keys (in a real app, these would come from Settings, we'll hardcode or let user provide them)
    void setFirebaseConfig(const QString &apiKey, const QString &dbUrl);

    // Auth
    void registerUser(const QString &email, const QString &password);
    void loginUser(const QString &email, const QString &password);
    void logout();
    bool isAuthenticated() const;
    QString currentUserEmail() const;
    
    // DB & Sync
    void uploadTrack(const TrackMetadata &meta);
    void fetchCloudTracks();
    void downloadTrackData(const QString &cloudId, const QString &destPath);

signals:
    void authSuccess(const QString &email);
    void authFailed(const QString &errorMsg);
    void tracksFetched(const QList<TrackMetadata> &tracks);
    void trackUploadProgress(const QString &title, int percent);
    void trackUploadFinished(const QString &title);
    void trackUploadFailed(const QString &title, const QString &errorMsg);
    void trackDownloadProgress(const QString &cloudId, int percent);
    void trackDownloaded(const QString &cloudId, const QString &localPath);
    void trackDownloadFailed(const QString &cloudId, const QString &errorMsg);

private:
    QNetworkAccessManager *m_network;
    QString m_apiKey;
    QString m_dbUrl;
    
    QString m_idToken;
    QString m_localId; // User ID
    QString m_email;

    void handleAuthResponse(QNetworkReply *reply, bool isLogin);
};

#endif // CLOUDMANAGER_H
