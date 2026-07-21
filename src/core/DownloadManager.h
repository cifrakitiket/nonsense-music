#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QProcess>

class DownloadManager : public QObject {
    Q_OBJECT
public:
    explicit DownloadManager(QObject *parent = nullptr);
    ~DownloadManager();

    void setYtDlpPath(const QString &path);
    QString ytDlpPath() const;

    void setDownloadDirectory(const QString &dir);
    QString downloadDirectory() const;

    // Bypass options
    void setUseCookiesFromBrowser(bool enabled);
    void setPlayerClient(const QString &client);
    void setUseByeDpi(bool enabled);
    void setByeDpiHost(const QString &host);
    void setGeoBypass(bool enabled);
    void setGeoBypassCountry(const QString &country);
    void setForceIPv4(bool enabled);
    void setForceIPv6(bool enabled);
    void setLegacyServerConnect(bool enabled);
    void setProxy(const QString &proxy);
    void setDownloadPlaylist(bool enabled);

    bool startDownload(const QString &url);
    void cancelDownload();
    bool isDownloading() const;

signals:
    void progressUpdated(int percentage);
    void statusUpdated(const QString &statusText);
    void downloadCompleted(const QString &filePath);
    void downloadPlaylistCompleted(const QStringList &filePaths);
    void downloadFailed(const QString &errorReason);

private slots:
    void readProcessOutput();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processError(QProcess::ProcessError error);

private:
    QProcess *m_process;
    QString m_ytDlpPath;
    QString m_downloadDir;
    QString m_downloadedFile;
    QString m_lastErrorMsg;
    QString m_stderrBuffer;
    bool m_isDownloading = false;

    // Bypass settings
    bool m_useCookies = false;
    QString m_playerClient;
    bool m_useByeDpi = false;
    QString m_byeDpiHost;
    bool m_geoBypass = false;
    QString m_geoBypassCountry;
    bool m_forceIPv4 = false;
    bool m_forceIPv6 = false;
    bool m_legacyServerConnect = false;
    QString m_proxy;
    bool m_downloadPlaylist = false;
    
    void parseOutputLine(const QString &line);
    void applyMetadataFromJson(const QString &mp3Path);
};

#endif // DOWNLOADMANAGER_H
