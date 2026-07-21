#include "mainwindow.h"
#include "EditTrackDialog.h"
#include "MiniPlayer.h"
#include "StyleManager.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QGraphicsBlurEffect>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QResizeEvent>
#include <QFrame>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QApplication>
#include <QDebug>
#include "IconProvider.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // Load saved theme BEFORE creating any UI
    StyleManager::instance(); // constructor loads from QSettings
    
    m_libraryManager = new LibraryManager(this);
    m_downloadManager = new DownloadManager(this);
    m_cloudManager = new CloudManager(this);
    m_cloudManager->setFirebaseConfig("AIzaSyBEpSo6lsL6GGr0-ccg5f-9SeXhmyxrdws", "https://nonsense-music-default-rtdb.asia-southeast1.firebasedatabase.app/");
    m_libraryManager->initDatabase();
    
    m_audioPlayer = new AudioPlayer(this);
    m_downloadManager = new DownloadManager(this);

    setupUI();
    applyGlobalStyle();
    
    connect(m_sidebar, &Sidebar::tabChanged, this, &MainWindow::onTabChanged);
    connect(m_settingsView, &SettingsView::languageChanged, this, &MainWindow::onLanguageChanged);
    connect(m_settingsView, &SettingsView::themeChanged, this, &MainWindow::onThemeChanged);
    connect(m_settingsView, &SettingsView::dynamicBackgroundToggled, this, &MainWindow::onDynamicBgToggled);
    connect(m_audioPlayer, &AudioPlayer::trackChanged, this, &MainWindow::onTrackChanged);
    
    // Create opacity effect ONCE on the stacked widget
    m_stackEffect = new QGraphicsOpacityEffect(m_stackedWidget);
    m_stackedWidget->setGraphicsEffect(m_stackEffect);
    m_stackEffect->setOpacity(1.0);
    
    m_stackFadeAnim = new QPropertyAnimation(m_stackEffect, "opacity", this);
    m_stackFadeAnim->setDuration(250);
    m_stackFadeAnim->setEasingCurve(QEasingCurve::OutCubic);
    
    // Apply saved language to all UI elements on startup
    retranslateUI();
    
    // Fetch cloud tracks if already logged in
    if (m_cloudManager->isAuthenticated()) {
        m_cloudManager->fetchCloudTracks();
    }
    
    // Setup System Tray
    setupTrayIcon();
}

MainWindow::~MainWindow() {}

void MainWindow::setupTrayIcon() {

    m_trayIcon = new QSystemTrayIcon(this);
    
    // Use an icon generated in code to 100% guarantee it's not a broken PNG/ICO file
    QPixmap trayPix = IconProvider::getPixmap(IconType::Note, QSize(64, 64), QColor("#1DB954"));
    m_trayIcon->setIcon(QIcon(trayPix));
    m_trayIcon->setToolTip("Nonsense Music");
    
    m_trayMenu = new QMenu(this);
    m_trayMenu->setStyleSheet(StyleManager::contextMenuStyle());
    
    QAction *playPauseAct = new QAction("Play/Pause", this);
    QAction *nextAct = new QAction("Next", this);
    QAction *prevAct = new QAction("Previous", this);
    QAction *toggleViewAct = new QAction("Show/Hide", this);
    QAction *quitAct = new QAction("Quit", this);
    
    connect(playPauseAct, &QAction::triggered, this, [this]() {
        if (m_audioPlayer) {
            if (m_audioPlayer->isPlaying()) m_audioPlayer->pause();
            else m_audioPlayer->play();
        }
    });
    connect(nextAct, &QAction::triggered, this, [this]() {
        if (m_audioPlayer) m_audioPlayer->next();
    });
    connect(prevAct, &QAction::triggered, this, [this]() {
        if (m_audioPlayer) m_audioPlayer->previous();
    });
    connect(toggleViewAct, &QAction::triggered, this, [this]() {
        if (isVisible()) hide();
        else { show(); activateWindow(); }
    });
    connect(quitAct, &QAction::triggered, this, [this]() {
        m_forceQuit = true;
        QApplication::quit();
    });
    
    m_trayMenu->addAction(playPauseAct);
    m_trayMenu->addAction(nextAct);
    m_trayMenu->addAction(prevAct);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(toggleViewAct);
    m_trayMenu->addAction(quitAct);
    
    m_trayIcon->setContextMenu(m_trayMenu);
    
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            if (isVisible()) hide();
            else { show(); activateWindow(); }
        }
    });
    
    m_trayIcon->show();
}

void MainWindow::setupUI() {
    setWindowTitle("Nonsense Music");
    setMinimumSize(950, 650);
    
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setObjectName("CentralWidget");
    setCentralWidget(centralWidget);
    
    m_bgLabel = new QLabel(centralWidget);
    m_bgLabel->setScaledContents(true);
    m_bgLabel->lower();
    
    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect(this);
    blur->setBlurRadius(80);
    m_bgLabel->setGraphicsEffect(blur);
    m_bgLabel->hide(); // Hidden by default, shown by applyGlobalStyle if enabled
    
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    m_sidebar = new Sidebar(m_libraryManager, this);
    // Width is set internally by Sidebar
    
    QWidget *rightContainer = new QWidget(this);
    rightContainer->setObjectName("RightContainer");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    
    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setObjectName("MainStackedWidget");
    
    m_libraryView = new LibraryView(m_libraryManager, this);
    m_downloadsView = new DownloadsView(m_downloadManager, m_libraryManager, this);
    m_authView = new AuthView(m_cloudManager, this);
    m_settingsView = new SettingsView(m_libraryManager, m_downloadManager, this);
    
    m_stackedWidget->addWidget(m_libraryView);   // index 0
    m_stackedWidget->addWidget(m_downloadsView); // index 1
    m_stackedWidget->addWidget(m_authView);      // index 2
    m_stackedWidget->addWidget(m_settingsView);  // index 3 
    
    m_playerBar = new PlayerBar(this);
    m_playerBar->setAudioPlayer(m_audioPlayer);
    
    rightLayout->addWidget(m_stackedWidget, 1);
    rightLayout->addWidget(m_playerBar);
    
    mainLayout->addWidget(m_sidebar);
    mainLayout->addWidget(rightContainer);
    
    connect(m_libraryView, &LibraryView::playRequested, this, &MainWindow::onPlayRequested);
    connect(m_libraryView, &LibraryView::editRequested, this, &MainWindow::onEditRequested);
    
    // Connect Sidebar library signals to LibraryView
    connect(m_sidebar, &Sidebar::playlistClicked, this, [this](int id) {
        m_sidebar->clearTabSelection();
        m_stackedWidget->setCurrentIndex(0); // Switch to LibraryView
        m_libraryView->openPlaylist(id);
    });
    
    connect(m_libraryView, &LibraryView::playRequested, this, [this](const TrackMetadata &track, const QList<TrackMetadata> &queue, int playlistId) {
        int startIdx = 0;
        for (int i = 0; i < queue.size(); ++i) {
            if (queue[i].filePath == track.filePath) {
                startIdx = i; break;
            }
        }
        m_audioPlayer->setPlaylist(queue, startIdx);
        m_currentPlaylistId = playlistId;
        m_sidebar->setCurrentPlayingPlaylist(playlistId);
    });

    connect(m_audioPlayer, &AudioPlayer::trackChanged, this, [this](const TrackMetadata &track) {
        m_libraryView->setCurrentPlayingTrack(track.filePath);
    });

    connect(m_sidebar, &Sidebar::trackClicked, this, [this](const TrackMetadata &track) {
        m_sidebar->clearTabSelection();
        m_audioPlayer->setPlaylist({track}, 0);
        m_currentPlaylistId = -1;
        m_sidebar->setCurrentPlayingPlaylist(-1);
    });

    connect(m_sidebar, &Sidebar::createPlaylistRequested, this, [this]() {
        QMetaObject::invokeMethod(m_libraryView, "onCreatePlaylistClicked");
    });
    connect(m_settingsView, &SettingsView::languageChanged, this, &MainWindow::onLanguageChanged);
    connect(m_playerBar, &PlayerBar::miniPlayerRequested, this, &MainWindow::onShowMiniPlayer);
    
    // Auth logic
    connect(m_authView, &AuthView::loginRequested, m_cloudManager, &CloudManager::loginUser);
    connect(m_authView, &AuthView::registerRequested, m_cloudManager, &CloudManager::registerUser);
    connect(m_authView, &AuthView::loginSuccessful, this, [this]() {
        // Go back to library on successful login
        m_sidebar->setCurrentTab(0);
        m_cloudManager->fetchCloudTracks();
    });
    connect(m_cloudManager, &CloudManager::tracksFetched, this, [this](const QList<TrackMetadata> &tracks) {
        for (const TrackMetadata &meta : tracks) {
            m_libraryManager->addCloudTrack(meta);
        }
    });
    
    connect(m_libraryView, &LibraryView::downloadRequested, this, [this](const TrackMetadata &track) {
        if (!track.isCloudOnly || track.cloudId.isEmpty()) return;
        
        QString destPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/cloud_tracks/";
        QDir().mkpath(destPath);
        QString safeTitle = track.title;
        safeTitle.replace(QRegularExpression("[\\\\/:*?\"<>|]"), "_");
        destPath += QFileInfo(safeTitle).baseName() + "_" + track.cloudId.left(8) + ".mp3";
        
        m_cloudManager->downloadTrackData(track.cloudId, destPath);
    });
    
    connect(m_cloudManager, &CloudManager::trackDownloaded, this, [this](const QString &cloudId, const QString &localPath) {
        m_libraryManager->convertCloudTrackToLocal(cloudId, localPath);
    });
    
    connect(m_libraryManager, &LibraryManager::trackAddedLocally, m_cloudManager, &CloudManager::uploadTrack);
}

void MainWindow::onTabChanged(int index) {
    m_stackFadeAnim->stop();
    m_stackEffect->setOpacity(0.0);
    
    m_stackedWidget->setCurrentIndex(index);
    
    if (index == 0) {
        m_libraryView->refresh();
    }
    
    m_stackFadeAnim->setStartValue(0.0);
    m_stackFadeAnim->setEndValue(1.0);
    m_stackFadeAnim->start();
}

void MainWindow::onPlayRequested(const TrackMetadata &track, const QList<TrackMetadata> &queue) {
    int startIdx = 0;
    for (int i = 0; i < queue.size(); ++i) {
        if (queue[i].filePath == track.filePath) {
            startIdx = i;
            break;
        }
    }
    m_audioPlayer->setPlaylist(queue, startIdx);
}

void MainWindow::onEditRequested(const TrackMetadata &track) {
    EditTrackDialog dialog(track, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_libraryManager->updateTrackInfo(track.filePath, dialog.title(), dialog.artist(), dialog.newCoverData());
        // If the currently playing track was edited, update the PlayerBar UI by
        // triggering a trackChanged signal from the audio player's current state.
        // Do NOT call setAudioPlayer() again \u2014 that would duplicate all signal connections.
        if (m_audioPlayer->currentTrack().filePath == track.filePath) {
            emit m_audioPlayer->trackChanged(m_audioPlayer->currentTrack());
        }
    }
}

void MainWindow::onLanguageChanged() {
    retranslateUI();
}

void MainWindow::onThemeChanged() {
    applyGlobalStyle();
    m_sidebar->refreshStyle();
    m_playerBar->refreshStyle();
    m_libraryView->refreshStyle();
    m_downloadsView->refreshStyle();
    m_settingsView->refreshStyle();
}

void MainWindow::onDynamicBgToggled() {
    applyGlobalStyle();
    onTrackChanged(m_audioPlayer->currentTrack());
}

void MainWindow::onTrackChanged(const TrackMetadata &track) {
    QPixmap pixmap;
    if (!track.coverData.isNull() && !track.coverData.isEmpty()) {
        pixmap.loadFromData(track.coverData);
    } else {
        QString coverPath = track.coverMimeType;
        if (coverPath.isEmpty() || !QFile::exists(coverPath)) {
            QByteArray hash = QCryptographicHash::hash(track.filePath.toUtf8(), QCryptographicHash::Md5);
            QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/covers/";
            coverPath = cacheDir + hash.toHex() + ".jpg";
        }
        if (QFile::exists(coverPath)) {
            pixmap.load(coverPath);
        }
    }

    if (!pixmap.isNull()) {
        // Scale down for blur performance and better spread
        pixmap = pixmap.scaled(256, 256, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        
        QPixmap tinted(pixmap.size());
        tinted.fill(Qt::transparent);
        QPainter p(&tinted);
        p.drawPixmap(0, 0, pixmap);
        // Add acrylic dark tint overlay for readability
        p.fillRect(tinted.rect(), QColor(0, 0, 0, 140));
        p.end();
        
        m_bgLabel->setPixmap(tinted);
    } else {
        m_bgLabel->clear();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    if (m_bgLabel) {
        m_bgLabel->resize(event->size());
    }
    QMainWindow::resizeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_forceQuit) {
        event->accept();
    } else {
        event->ignore();
        hide();
        m_trayIcon->showMessage("Nonsense Music", "App is still running in the background.", QSystemTrayIcon::Information, 2000);
    }
}

void MainWindow::retranslateUI() {
    m_sidebar->retranslateUI();
    m_libraryView->retranslateUI();
    m_downloadsView->retranslateUI();
    m_settingsView->retranslateUI();
}

void MainWindow::applyGlobalStyle() {
    QSettings settings("NonsenseMusic", "Player");
    bool dynBg = settings.value("dynamicBackground", true).toBool();
    
    if (dynBg) {
        m_bgLabel->show();
        QColor primaryColor(StyleManager::bgPrimary());
        QColor sidebarColor(StyleManager::bgSidebar());
        
        setStyleSheet(QString(
            "QMainWindow { background: transparent; }"
            "QWidget#CentralWidget { background: transparent; }"
            "QWidget#RightContainer { background-color: rgba(%1, %2, %3, %4); }"
            "QStackedWidget#MainStackedWidget { background: transparent; }"
        ).arg(primaryColor.red()).arg(primaryColor.green()).arg(primaryColor.blue()).arg(210)); // ~82% opacity
        
        m_sidebar->setStyleSheet(QString("QWidget#SpotifySidebar { background-color: rgba(%1, %2, %3, %4); }")
            .arg(sidebarColor.red()).arg(sidebarColor.green()).arg(sidebarColor.blue()).arg(180)); // ~70% opacity
    } else {
        m_bgLabel->hide();
        setStyleSheet(QString(
            "QMainWindow { background-color: %1; }"
            "QWidget#CentralWidget { background-color: %1; }"
            "QWidget#RightContainer { background-color: %1; }"
            "QStackedWidget#MainStackedWidget { background-color: %1; }"
        ).arg(StyleManager::bgPrimary()));
        
        m_sidebar->setStyleSheet(QString("QWidget#SpotifySidebar { background-color: %1; }")
            .arg(StyleManager::bgSidebar()));
    }
}

void MainWindow::onShowMiniPlayer() {
    if (!m_miniPlayer) {
        m_miniPlayer = new MiniPlayer(m_audioPlayer);
        m_miniPlayer->setMainWindow(this);
    }
    
    m_miniPlayer->show();
}
