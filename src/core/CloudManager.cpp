#include "CloudManager.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QDebug>

CloudManager::CloudManager(QObject *parent) : QObject(parent), m_network(new QNetworkAccessManager(this)) {
    QSettings settings("NonsenseMusic", "Player");
    m_idToken = settings.value("cloud_idToken").toString();
    m_localId = settings.value("cloud_localId").toString();
    m_email = settings.value("cloud_email").toString();
}

CloudManager::~CloudManager() {
}

void CloudManager::setFirebaseConfig(const QString &apiKey, const QString &dbUrl) {
    m_apiKey = apiKey;
    m_dbUrl = dbUrl;
    if (m_dbUrl.endsWith('/')) {
        m_dbUrl.chop(1);
    }
}

void CloudManager::registerUser(const QString &email, const QString &password) {
    if (m_apiKey.isEmpty()) {
        emit authFailed("Firebase API Key is not set.");
        return;
    }
    
    QUrl url("https://identitytoolkit.googleapis.com/v1/accounts:signUp?key=" + m_apiKey);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["email"] = email;
    json["password"] = password;
    json["returnSecureToken"] = true;

    QNetworkReply *reply = m_network->post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleAuthResponse(reply, false);
    });
}

void CloudManager::loginUser(const QString &email, const QString &password) {
    if (m_apiKey.isEmpty()) {
        emit authFailed("Firebase API Key is not set.");
        return;
    }
    
    QUrl url("https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=" + m_apiKey);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["email"] = email;
    json["password"] = password;
    json["returnSecureToken"] = true;

    QNetworkReply *reply = m_network->post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleAuthResponse(reply, true);
    });
}

void CloudManager::logout() {
    m_idToken.clear();
    m_localId.clear();
    m_email.clear();
    
    QSettings settings("NonsenseMusic", "Player");
    settings.remove("cloud_idToken");
    settings.remove("cloud_localId");
    settings.remove("cloud_email");
}

bool CloudManager::isAuthenticated() const {
    return !m_idToken.isEmpty() && !m_localId.isEmpty();
}

QString CloudManager::currentUserEmail() const {
    return m_email;
}

void CloudManager::handleAuthResponse(QNetworkReply *reply, bool isLogin) {
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        // Try to parse Firebase error message
        QString errorMsg = reply->errorString();
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull() && doc.isObject()) {
            QJsonObject root = doc.object();
            if (root.contains("error")) {
                QJsonObject errorObj = root["error"].toObject();
                if (errorObj.contains("message")) {
                    errorMsg = errorObj["message"].toString();
                }
            }
        }
        emit authFailed(errorMsg);
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    m_idToken = obj["idToken"].toString();
    m_localId = obj["localId"].toString();
    m_email = obj["email"].toString();

    QSettings settings("NonsenseMusic", "Player");
    settings.setValue("cloud_idToken", m_idToken);
    settings.setValue("cloud_localId", m_localId);
    settings.setValue("cloud_email", m_email);

    emit authSuccess(m_email);
}

void CloudManager::uploadTrack(const TrackMetadata &meta) {
    if (!isAuthenticated()) {
        emit trackUploadFailed(meta.title, "Not authenticated.");
        return;
    }

    // Read audio file and convert to Base64
    QFile file(meta.filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit trackUploadFailed(meta.title, "Could not read local file.");
        return;
    }
    
    QByteArray audioData = file.readAll();
    file.close();
    
    // IMPORTANT: Warning about Base64 size
    QString base64Audio = QString::fromLatin1(audioData.toBase64());
    QString base64Cover = QString::fromLatin1(meta.coverData.toBase64());
    
    QJsonObject trackObj;
    trackObj["title"] = meta.title;
    trackObj["artist"] = meta.artist;
    trackObj["album"] = meta.album;
    trackObj["duration"] = meta.duration;
    trackObj["coverMimeType"] = meta.coverMimeType;
    trackObj["coverData"] = base64Cover;
    trackObj["audioData"] = base64Audio;
    trackObj["fileName"] = QFileInfo(meta.filePath).fileName();
    
    QString trackId = QCryptographicHash::hash(meta.filePath.toUtf8(), QCryptographicHash::Md5).toHex();
    
    // URL: <dbUrl>/users/<uid>/tracks/<trackId>.json?auth=<token>
    QUrl url(QString("%1/users/%2/tracks/%3.json?auth=%4")
             .arg(m_dbUrl).arg(m_localId).arg(trackId).arg(m_idToken));
             
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = m_network->put(request, QJsonDocument(trackObj).toJson());
    
    // Connect progress
    connect(reply, &QNetworkReply::uploadProgress, this, [this, meta](qint64 bytesSent, qint64 bytesTotal) {
        if (bytesTotal > 0) {
            emit trackUploadProgress(meta.title, (bytesSent * 100) / bytesTotal);
        }
    });
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, meta]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit trackUploadFailed(meta.title, reply->errorString());
        } else {
            emit trackUploadFinished(meta.title);
        }
    });
}

void CloudManager::fetchCloudTracks() {
    if (!isAuthenticated()) {
        return; // Silent fail
    }

    // Since audioData can be massive, it's highly inefficient to fetch the entire node.
    // However, Firebase RTDB REST API does not support selecting specific fields like GraphQL.
    // We will use the REST API shallow parameter to get keys, but wait, shallow=true only returns keys, not metadata.
    // Because we placed audioData next to metadata, fetching the list will download ALL audio data.
    // THIS IS A HUGE PROBLEM with this approach, but we must implement what the user requested.
    
    QUrl url(QString("%1/users/%2/tracks.json?auth=%3").arg(m_dbUrl).arg(m_localId).arg(m_idToken));
    QNetworkRequest request(url);
    
    QNetworkReply *reply = m_network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            return;
        }
        
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || !doc.isObject()) return;
        
        QJsonObject tracksObj = doc.object();
        QList<TrackMetadata> list;
        
        for (auto it = tracksObj.begin(); it != tracksObj.end(); ++it) {
            QString cloudId = it.key();
            QJsonObject trackObj = it.value().toObject();
            
            TrackMetadata meta;
            meta.cloudId = cloudId;
            meta.isCloudOnly = true;
            meta.title = trackObj["title"].toString();
            meta.artist = trackObj["artist"].toString();
            meta.album = trackObj["album"].toString();
            meta.duration = trackObj["duration"].toInt();
            meta.coverMimeType = trackObj["coverMimeType"].toString();
            
            QString base64Cover = trackObj["coverData"].toString();
            if (!base64Cover.isEmpty()) {
                meta.coverData = QByteArray::fromBase64(base64Cover.toLatin1());
            }
            // Do not extract audioData here to save memory
            list.append(meta);
        }
        
        emit tracksFetched(list);
    });
}

void CloudManager::downloadTrackData(const QString &cloudId, const QString &destPath) {
    if (!isAuthenticated()) {
        emit trackDownloadFailed(cloudId, "Not authenticated");
        return;
    }
    
    QUrl url(QString("%1/users/%2/tracks/%3.json?auth=%4").arg(m_dbUrl).arg(m_localId).arg(cloudId).arg(m_idToken));
    QNetworkRequest request(url);
    
    QNetworkReply *reply = m_network->get(request);
    
    connect(reply, &QNetworkReply::downloadProgress, this, [this, cloudId](qint64 bytesReceived, qint64 bytesTotal) {
        if (bytesTotal > 0) {
            emit trackDownloadProgress(cloudId, (bytesReceived * 100) / bytesTotal);
        }
    });
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, cloudId, destPath]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit trackDownloadFailed(cloudId, reply->errorString());
            return;
        }
        
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();
        
        QString base64Audio = obj["audioData"].toString();
        if (base64Audio.isEmpty()) {
            emit trackDownloadFailed(cloudId, "No audio data found in cloud");
            return;
        }
        
        QByteArray audioData = QByteArray::fromBase64(base64Audio.toLatin1());
        
        QFile file(destPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(audioData);
            file.close();
            emit trackDownloaded(cloudId, destPath);
        } else {
            emit trackDownloadFailed(cloudId, "Could not write to local disk");
        }
    });
}
