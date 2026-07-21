#include "DownloadManager.h"
#include "MetadataManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QRegularExpression>
#include <QCoreApplication>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDebug>

DownloadManager::DownloadManager(QObject *parent) : QObject(parent) {
    m_process = new QProcess(this);
    
    connect(m_process, &QProcess::readyReadStandardOutput, this, &DownloadManager::readProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &DownloadManager::readProcessOutput);
    
    connect(m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), 
            this, &DownloadManager::processFinished);
            
    connect(m_process, &QProcess::errorOccurred, this, &DownloadManager::processError);
    
    // Look for yt-dlp.exe in the application directory where it was installed
    m_ytDlpPath = QCoreApplication::applicationDirPath() + "/yt-dlp.exe";
    
    QString musicPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    if (musicPath.isEmpty()) {
        musicPath = QDir::homePath() + "/Music";
    }
    m_downloadDir = musicPath + "/NonsenseMusic";
    QDir().mkpath(m_downloadDir);
}

DownloadManager::~DownloadManager() {
    cancelDownload();
}

void DownloadManager::setYtDlpPath(const QString &path) {
    m_ytDlpPath = path;
}

QString DownloadManager::ytDlpPath() const {
    return m_ytDlpPath;
}

void DownloadManager::setDownloadDirectory(const QString &dir) {
    m_downloadDir = dir;
    QDir().mkpath(m_downloadDir);
}

QString DownloadManager::downloadDirectory() const {
    return m_downloadDir;
}

void DownloadManager::setUseCookiesFromBrowser(bool enabled) { m_useCookies = enabled; }
void DownloadManager::setPlayerClient(const QString &client) { m_playerClient = client; }
void DownloadManager::setUseByeDpi(bool enabled) { m_useByeDpi = enabled; }
void DownloadManager::setByeDpiHost(const QString &host) { m_byeDpiHost = host; }
void DownloadManager::setGeoBypass(bool enabled) { m_geoBypass = enabled; }
void DownloadManager::setGeoBypassCountry(const QString &country) { m_geoBypassCountry = country; }
void DownloadManager::setForceIPv4(bool enabled) { m_forceIPv4 = enabled; }
void DownloadManager::setForceIPv6(bool enabled) { m_forceIPv6 = enabled; }
void DownloadManager::setLegacyServerConnect(bool enabled) { m_legacyServerConnect = enabled; }
void DownloadManager::setProxy(const QString &proxy) { m_proxy = proxy; }

bool DownloadManager::startDownload(const QString &url) {
    if (m_isDownloading) {
        emit downloadFailed("Download already in progress.");
        return false;
    }
    
    if (url.trimmed().isEmpty()) {
        emit downloadFailed("URL is empty.");
        return false;
    }
    
    if (!QFile::exists(m_ytDlpPath)) {
        emit downloadFailed("yt-dlp.exe not found in application directory.");
        return false;
    }
    
    m_downloadedFile.clear();
    m_lastErrorMsg.clear();
    m_stderrBuffer.clear();
    m_isDownloading = true;
    
    emit statusUpdated("Starting download...");
    
    QStringList args;

    // Mimic Chrome
    args << "--user-agent" << "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"
         << "--referer" << "https://www.youtube.com/"
         << "--add-header" << "Accept-Language: en-US,en;q=0.9"
         << "--add-header" << "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8";

    // Bypass: cookies from Chrome browser
    if (m_useCookies) {
        args << "--cookies-from-browser" << "chrome";
    }

    // Bypass: player client selection
    if (!m_playerClient.isEmpty()) {
        args << "--extractor-args" << "youtube:player_client=" + m_playerClient;
    }

    // Bypass: proxy (supports socks5://, http://, etc.)
    if (!m_proxy.isEmpty()) {
        args << "--proxy" << m_proxy;
    }

    // Bypass: geo-bypass
    if (m_geoBypass) {
        args << "--geo-bypass";
        if (!m_geoBypassCountry.isEmpty()) {
            args << "--geo-bypass-country" << m_geoBypassCountry;
        }
    }

    // Bypass: force IPv4/IPv6
    if (m_forceIPv4) {
        args << "--force-ipv4";
    }
    if (m_forceIPv6) {
        args << "--force-ipv6";
    }

    // Bypass: legacy server connect (old SSL certs)
    if (m_legacyServerConnect) {
        args << "--legacy-server-connect";
    }

    // Download best audio, extract as mp3, write metadata JSON for artist extraction
    args << "-x"
         << "--audio-format" << "mp3"
         << "--audio-quality" << "0"
         << "--embed-thumbnail"
         << "--write-info-json"
         << "--no-playlist"
         << "-o" << m_downloadDir + "/%(title)s.%(ext)s"
         << url;
         
    m_process->start(m_ytDlpPath, args);
    return true;
}

void DownloadManager::cancelDownload() {
    if (m_isDownloading) {
        m_process->kill();
        m_isDownloading = false;
        emit statusUpdated("Download cancelled.");
        emit downloadFailed("Cancelled by user.");
    }
}

bool DownloadManager::isDownloading() const {
    return m_isDownloading;
}

void DownloadManager::readProcessOutput() {
    m_process->setReadChannel(QProcess::StandardOutput);
    while (m_process->canReadLine()) {
        QString line = QString::fromLocal8Bit(m_process->readLine()).trimmed();
        parseOutputLine(line);
    }
    
    m_process->setReadChannel(QProcess::StandardError);
    while (m_process->canReadLine()) {
        QString line = QString::fromLocal8Bit(m_process->readLine()).trimmed();
        m_stderrBuffer += line + "\n";
        parseOutputLine(line);
    }
}

void DownloadManager::parseOutputLine(const QString &line) {
    if (line.isEmpty()) return;
    
    qDebug() << "yt-dlp:" << line;
    
    if (line.startsWith("ERROR:")) {
        if (!m_lastErrorMsg.isEmpty()) m_lastErrorMsg += "\n";
        m_lastErrorMsg += line;
    }
    
    if (line.contains("[download]") && line.contains("%")) {
        static QRegularExpression pctRx(R"(\b(\d+(?:\.\d+)?)%)");
        QRegularExpressionMatch match = pctRx.match(line);
        if (match.hasMatch()) {
            double percent = match.captured(1).toDouble();
            emit progressUpdated(qRound(percent));
            emit statusUpdated(QString("Downloading: %1%").arg(qRound(percent)));
        }
    }
    
    if (line.contains("[ExtractAudio]")) {
        emit statusUpdated("Converting to MP3...");
    }
    
    if (line.contains("Destination:") && line.endsWith(".mp3")) {
        int idx = line.indexOf("Destination:");
        if (idx != -1) {
            QString path = line.mid(idx + 12).trimmed();
            m_downloadedFile = QDir::toNativeSeparators(path);
        }
    }
}

void DownloadManager::applyMetadataFromJson(const QString &mp3Path) {
    // Look for the .info.json file next to the MP3
    QString jsonPath = mp3Path;
    jsonPath.chop(4); // remove .mp3
    jsonPath += ".info.json";
    
    if (!QFile::exists(jsonPath)) {
        qDebug() << "No .info.json found at" << jsonPath;
        return;
    }
    
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open .info.json:" << jsonPath;
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        qDebug() << ".info.json is not a valid JSON object";
        QFile::remove(jsonPath);
        return;
    }
    
    QJsonObject obj = doc.object();
    
    // Extract uploader (channel name) as artist
    QString uploader = obj.value("uploader").toString();
    QString title = obj.value("title").toString();
    
    if (uploader.isEmpty()) {
        uploader = obj.value("channel").toString();
    }
    
    qDebug() << "Extracted from JSON - title:" << title << "uploader:" << uploader;
    
    if (!uploader.isEmpty()) {
        // Write artist tag to the MP3 file using TagLib
        MetadataManager::writeMetadata(mp3Path, title, uploader, QByteArray());
        qDebug() << "Wrote artist tag:" << uploader << "to" << mp3Path;
    }
    
    // Clean up the .info.json file
    QFile::remove(jsonPath);
}

void DownloadManager::processFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    m_isDownloading = false;
    
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        if (!m_downloadedFile.isEmpty() && QFile::exists(m_downloadedFile)) {
            // Apply metadata from .info.json (uploader as artist)
            applyMetadataFromJson(m_downloadedFile);
            
            emit progressUpdated(100);
            emit statusUpdated("Download and conversion complete!");
            emit downloadCompleted(m_downloadedFile);
        } else {
            // Fallback: scan directory for newest mp3
            QDir dir(m_downloadDir);
            dir.setNameFilters(QStringList() << "*.mp3");
            dir.setSorting(QDir::Time);
            QFileInfoList list = dir.entryInfoList();
            if (!list.isEmpty()) {
                QString newestFile = list.first().absoluteFilePath();
                
                // Apply metadata from .info.json
                applyMetadataFromJson(newestFile);
                
                emit progressUpdated(100);
                emit statusUpdated("Download complete!");
                emit downloadCompleted(newestFile);
            } else {
                emit downloadFailed("Could not locate downloaded MP3 file.");
            }
        }
    } else {
        QString errorMsg = m_lastErrorMsg;
        if (errorMsg.isEmpty()) {
            errorMsg = m_stderrBuffer.trimmed();
        }
        if (errorMsg.isEmpty()) {
            errorMsg = "yt-dlp process exited with error code " + QString::number(exitCode);
        }
        emit downloadFailed(errorMsg);
    }
}

void DownloadManager::processError(QProcess::ProcessError error) {
    m_isDownloading = false;
    QString errorText;
    switch (error) {
        case QProcess::FailedToStart:
            errorText = "Failed to start yt-dlp. Make sure it is installed and the path is correct.";
            break;
        case QProcess::Crashed:
            errorText = "yt-dlp process crashed.";
            break;
        case QProcess::Timedout:
            errorText = "Download process timed out.";
            break;
        default:
            errorText = "An error occurred during the download process.";
            break;
    }
    emit downloadFailed(errorText);
}
