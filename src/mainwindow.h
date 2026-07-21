#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCloseEvent>
#include "Sidebar.h"
#include "PlayerBar.h"
#include "LibraryView.h"
#include "DownloadsView.h"
#include "SettingsView.h"
#include "CloudManager.h"
#include "AuthView.h"
#include "AudioPlayer.h"
#include "LibraryManager.h"
#include "DownloadManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTabChanged(int index);
    void onPlayRequested(const TrackMetadata &track, const QList<TrackMetadata> &queue);
    void onEditRequested(const TrackMetadata &track);
    void onLanguageChanged();
    void onThemeChanged();
    void onDynamicBgToggled();
    void onTrackChanged(const TrackMetadata &track);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    AudioPlayer *m_audioPlayer;
    LibraryManager *m_libraryManager;
    DownloadManager *m_downloadManager;
    CloudManager *m_cloudManager;

    Sidebar *m_sidebar;
    PlayerBar *m_playerBar;
    QStackedWidget *m_stackedWidget;
    
    LibraryView *m_libraryView;
    DownloadsView *m_downloadsView;
    SettingsView *m_settingsView;
    AuthView *m_authView;
    class MiniPlayer *m_miniPlayer = nullptr;

    int m_currentPlaylistId = -1;

    QLabel *m_bgLabel = nullptr;

    QGraphicsOpacityEffect *m_stackEffect = nullptr;
    QPropertyAnimation *m_stackFadeAnim = nullptr;

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    bool m_forceQuit = false;

    void setupUI();
    void setupTrayIcon();
    void retranslateUI();
    void applyGlobalStyle();
private slots:
    void onShowMiniPlayer();
};

#endif // MAINWINDOW_H
