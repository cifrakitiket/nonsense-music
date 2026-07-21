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

    bool startDownload(const QString &url);
    void cancelDownload();
    bool isDownloading() const;

signals:
    void progressUpdated(int percentage);
    void statusUpdated(const QString &statusText);
    void downloadCompleted(const QString &filePath);
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
    
    void parseOutputLine(const QString &line);
    void applyMetadataFromJson(const QString &mp3Path);
};

#endif // DOWNLOADMANAGER_H
