#ifndef DOWNLOADSVIEW_H
#define DOWNLOADSVIEW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include "DownloadManager.h"
#include "LibraryManager.h"

class DownloadsView : public QWidget {
    Q_OBJECT
public:
    explicit DownloadsView(DownloadManager *dlMgr, LibraryManager *libMgr, QWidget *parent = nullptr);
    ~DownloadsView();

    void retranslateUI();
    void refreshStyle();

private slots:
    void onDownloadClicked();
    void onCancelClicked();

    // Download Manager slots
    void onProgressUpdated(int percent);
    void onStatusUpdated(const QString &text);
    void onDownloadCompleted(const QString &filePath);
    void onDownloadPlaylistCompleted(const QStringList &filePaths);
    void onDownloadFailed(const QString &error);

private:
    DownloadManager *m_downloadManager;
    LibraryManager *m_libraryManager;

    QLabel *m_titleLabel;
    QLabel *m_subTitleLabel;
    QLineEdit *m_urlInput;
    QPushButton *m_downloadBtn;
    QPushButton *m_cancelBtn;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;

    // Bypass UI
    QCheckBox *m_playlistCheck;
    QCheckBox *m_cookiesCheck;
    QCheckBox *m_byedpiCheck;
    QLineEdit *m_byedpiHostInput;
    QComboBox *m_playerClientCombo;
    QCheckBox *m_geoBypassCheck;
    QLineEdit *m_geoCountryInput;
    QCheckBox *m_forceIPv4Check;
    QCheckBox *m_forceIPv6Check;
    QCheckBox *m_legacySslCheck;
    QLineEdit *m_proxyInput;
    QLabel *m_bypassHeaderLabel;

    QString m_lastDownloadedPath;

    void setupUI();
    void applyQSS();
    void loadBypassSettings();
    void saveBypassSettings();
    void showPlaylistSelector(const QString &filePath);
};

#endif // DOWNLOADSVIEW_H
