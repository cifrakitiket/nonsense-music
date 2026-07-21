#include "DownloadsView.h"
#include "TranslationManager.h"
#include "StyleManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QDialog>
#include <QListWidget>
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>
#include <QSettings>

DownloadsView::DownloadsView(DownloadManager *dlMgr, LibraryManager *libMgr, QWidget *parent)
    : QWidget(parent), m_downloadManager(dlMgr), m_libraryManager(libMgr) {
    setupUI();
    applyQSS();
    loadBypassSettings();

    connect(m_downloadBtn, &QPushButton::clicked, this, &DownloadsView::onDownloadClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &DownloadsView::onCancelClicked);

    connect(m_downloadManager, &DownloadManager::progressUpdated, this, &DownloadsView::onProgressUpdated);
    connect(m_downloadManager, &DownloadManager::statusUpdated, this, &DownloadsView::onStatusUpdated);
    connect(m_downloadManager, &DownloadManager::downloadCompleted, this, &DownloadsView::onDownloadCompleted);
    connect(m_downloadManager, &DownloadManager::downloadFailed, this, &DownloadsView::onDownloadFailed);

    // Bypass UI connections
    connect(m_cookiesCheck, &QCheckBox::toggled, this, &DownloadsView::saveBypassSettings);
    connect(m_byedpiCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_byedpiHostInput->setEnabled(checked);
        saveBypassSettings();
    });
    connect(m_byedpiHostInput, &QLineEdit::editingFinished, this, &DownloadsView::saveBypassSettings);
    connect(m_playerClientCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &DownloadsView::saveBypassSettings);
    connect(m_geoBypassCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_geoCountryInput->setEnabled(checked);
        saveBypassSettings();
    });
    connect(m_geoCountryInput, &QLineEdit::editingFinished, this, &DownloadsView::saveBypassSettings);
    connect(m_forceIPv4Check, &QCheckBox::toggled, this, &DownloadsView::saveBypassSettings);
    connect(m_forceIPv6Check, &QCheckBox::toggled, this, &DownloadsView::saveBypassSettings);
    connect(m_legacySslCheck, &QCheckBox::toggled, this, &DownloadsView::saveBypassSettings);
    connect(m_proxyInput, &QLineEdit::editingFinished, this, &DownloadsView::saveBypassSettings);
}

DownloadsView::~DownloadsView() {}

void DownloadsView::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 12);
    mainLayout->setSpacing(12);
    mainLayout->setAlignment(Qt::AlignTop);
    
    m_titleLabel = new QLabel(trL("dl_title"), this);
    m_titleLabel->setStyleSheet(QString("font-size: 24px; font-weight: bold; color: %1;").arg(StyleManager::textPrimary()));
    mainLayout->addWidget(m_titleLabel);
    
    m_subTitleLabel = new QLabel(trL("dl_subtitle"), this);
    m_subTitleLabel->setStyleSheet(QString("font-size: 13px; color: %1;").arg(StyleManager::textSecondary()));
    mainLayout->addWidget(m_subTitleLabel);
    
    m_urlInput = new QLineEdit(this);
    m_urlInput->setPlaceholderText("https://www.youtube.com/watch?v=...");
    m_urlInput->setMinimumHeight(36);
    m_urlInput->setMaximumHeight(44);
    m_urlInput->setStyleSheet(StyleManager::inputStyle());
    mainLayout->addWidget(m_urlInput);
    
    QWidget *controlRow = new QWidget(this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlRow);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->setSpacing(12);
    
    m_downloadBtn = new QPushButton(trL("btn_download"), this);
    m_downloadBtn->setMinimumHeight(36);
    m_downloadBtn->setMaximumHeight(44);
    m_downloadBtn->setMinimumWidth(120);
    m_downloadBtn->setCursor(Qt::PointingHandCursor);

    m_cancelBtn = new QPushButton(trL("btn_cancel"), this);
    m_cancelBtn->setMinimumHeight(36);
    m_cancelBtn->setMaximumHeight(44);
    m_cancelBtn->setMinimumWidth(100);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setEnabled(false);
    
    controlLayout->addWidget(m_downloadBtn);
    controlLayout->addWidget(m_cancelBtn);
    controlLayout->addStretch();
    
    mainLayout->addWidget(controlRow);
    
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    
    m_statusLabel = new QLabel("", this);
    m_statusLabel->setStyleSheet(QString("font-size: 13px; color: %1;").arg(StyleManager::textSecondary()));
    
    mainLayout->addWidget(m_progressBar);
    mainLayout->addWidget(m_statusLabel);

    // --- Bypass Section ---
    m_bypassHeaderLabel = new QLabel(trL("dl_bypass_header"), this);
    m_bypassHeaderLabel->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1; margin-top: 12px;").arg(StyleManager::textPrimary()));
    mainLayout->addWidget(m_bypassHeaderLabel);

    // Cookies from browser
    m_cookiesCheck = new QCheckBox(trL("dl_use_cookies"), this);
    m_cookiesCheck->setCursor(Qt::PointingHandCursor);
    mainLayout->addWidget(m_cookiesCheck);

    // Player client
    QWidget *playerClientRow = new QWidget(this);
    QHBoxLayout *pcLayout = new QHBoxLayout(playerClientRow);
    pcLayout->setContentsMargins(0, 0, 0, 0);
    pcLayout->setSpacing(12);
    QLabel *pcLabel = new QLabel(trL("dl_player_client"), this);
    pcLabel->setStyleSheet(QString("color: %1; font-size: 13px;").arg(StyleManager::textSecondary()));
    m_playerClientCombo = new QComboBox(this);
    m_playerClientCombo->addItems(QStringList() << trL("dl_pc_default") << "web" << "ios" << "android" << "mweb" << "tv" << "tv_embedded" << "mediaconnect");
    m_playerClientCombo->setMinimumHeight(30);
    pcLayout->addWidget(pcLabel);
    pcLayout->addWidget(m_playerClientCombo);
    pcLayout->addStretch();
    mainLayout->addWidget(playerClientRow);

    // ByeDPI
    QWidget *byedpiRow = new QWidget(this);
    QHBoxLayout *bdLayout = new QHBoxLayout(byedpiRow);
    bdLayout->setContentsMargins(0, 0, 0, 0);
    bdLayout->setSpacing(12);
    m_byedpiCheck = new QCheckBox(trL("dl_use_byedpi"), this);
    m_byedpiCheck->setCursor(Qt::PointingHandCursor);
    m_byedpiHostInput = new QLineEdit(this);
    m_byedpiHostInput->setPlaceholderText("127.0.0.1:8080");
    m_byedpiHostInput->setMinimumHeight(30);
    m_byedpiHostInput->setMaximumWidth(200);
    m_byedpiHostInput->setEnabled(false);
    bdLayout->addWidget(m_byedpiCheck);
    bdLayout->addWidget(m_byedpiHostInput);
    bdLayout->addStretch();
    mainLayout->addWidget(byedpiRow);

    // Geo bypass
    QWidget *geoRow = new QWidget(this);
    QHBoxLayout *geoLayout = new QHBoxLayout(geoRow);
    geoLayout->setContentsMargins(0, 0, 0, 0);
    geoLayout->setSpacing(12);
    m_geoBypassCheck = new QCheckBox(trL("dl_geo_bypass"), this);
    m_geoBypassCheck->setCursor(Qt::PointingHandCursor);
    m_geoCountryInput = new QLineEdit(this);
    m_geoCountryInput->setPlaceholderText(trL("dl_geo_country_hint"));
    m_geoCountryInput->setMinimumHeight(30);
    m_geoCountryInput->setMaximumWidth(80);
    m_geoCountryInput->setEnabled(false);
    geoLayout->addWidget(m_geoBypassCheck);
    geoLayout->addWidget(m_geoCountryInput);
    geoLayout->addStretch();
    mainLayout->addWidget(geoRow);

    // IPv4 / IPv6 / Legacy SSL
    QWidget *netRow = new QWidget(this);
    QHBoxLayout *netLayout = new QHBoxLayout(netRow);
    netLayout->setContentsMargins(0, 0, 0, 0);
    netLayout->setSpacing(16);
    m_forceIPv4Check = new QCheckBox("IPv4", this);
    m_forceIPv4Check->setCursor(Qt::PointingHandCursor);
    m_forceIPv6Check = new QCheckBox("IPv6", this);
    m_forceIPv6Check->setCursor(Qt::PointingHandCursor);
    m_legacySslCheck = new QCheckBox(trL("dl_legacy_ssl"), this);
    m_legacySslCheck->setCursor(Qt::PointingHandCursor);
    netLayout->addWidget(m_forceIPv4Check);
    netLayout->addWidget(m_forceIPv6Check);
    netLayout->addWidget(m_legacySslCheck);
    netLayout->addStretch();
    mainLayout->addWidget(netRow);

    // Proxy
    QWidget *proxyRow = new QWidget(this);
    QHBoxLayout *proxyLayout = new QHBoxLayout(proxyRow);
    proxyLayout->setContentsMargins(0, 0, 0, 0);
    proxyLayout->setSpacing(12);
    QLabel *proxyLabel = new QLabel(trL("dl_proxy"), this);
    proxyLabel->setStyleSheet(QString("color: %1; font-size: 13px;").arg(StyleManager::textSecondary()));
    m_proxyInput = new QLineEdit(this);
    m_proxyInput->setPlaceholderText("socks5://127.0.0.1:1080");
    m_proxyInput->setMinimumHeight(30);
    proxyLayout->addWidget(proxyLabel);
    proxyLayout->addWidget(m_proxyInput);
    proxyLayout->addStretch();
    mainLayout->addWidget(proxyRow);
}

void DownloadsView::retranslateUI() {
    m_titleLabel->setText(trL("dl_title"));
    m_subTitleLabel->setText(trL("dl_subtitle"));
    m_downloadBtn->setText(trL("btn_download"));
    m_cancelBtn->setText(trL("btn_cancel"));
    m_bypassHeaderLabel->setText(trL("dl_bypass_header"));
    m_cookiesCheck->setText(trL("dl_use_cookies"));
    m_byedpiCheck->setText(trL("dl_use_byedpi"));
    m_geoBypassCheck->setText(trL("dl_geo_bypass"));
    m_geoCountryInput->setPlaceholderText(trL("dl_geo_country_hint"));
    m_legacySslCheck->setText(trL("dl_legacy_ssl"));
    m_proxyInput->setPlaceholderText("socks5://127.0.0.1:1080");
}

void DownloadsView::loadBypassSettings() {
    QSettings s("NonsenseMusic", "Player");
    m_cookiesCheck->setChecked(s.value("bypass/cookies", false).toBool());
    int pcIdx = s.value("bypass/playerClient", 0).toInt();
    if (pcIdx >= 0 && pcIdx < m_playerClientCombo->count())
        m_playerClientCombo->setCurrentIndex(pcIdx);
    m_byedpiCheck->setChecked(s.value("bypass/byedpi", false).toBool());
    m_byedpiHostInput->setText(s.value("bypass/byedpiHost", "127.0.0.1:8080").toString());
    m_byedpiHostInput->setEnabled(m_byedpiCheck->isChecked());
    m_geoBypassCheck->setChecked(s.value("bypass/geoBypass", false).toBool());
    m_geoCountryInput->setText(s.value("bypass/geoCountry", "").toString());
    m_geoCountryInput->setEnabled(m_geoBypassCheck->isChecked());
    m_forceIPv4Check->setChecked(s.value("bypass/forceIPv4", false).toBool());
    m_forceIPv6Check->setChecked(s.value("bypass/forceIPv6", false).toBool());
    m_legacySslCheck->setChecked(s.value("bypass/legacySsl", false).toBool());
    m_proxyInput->setText(s.value("bypass/proxy", "").toString());
}

void DownloadsView::saveBypassSettings() {
    QSettings s("NonsenseMusic", "Player");
    s.setValue("bypass/cookies", m_cookiesCheck->isChecked());
    s.setValue("bypass/playerClient", m_playerClientCombo->currentIndex());
    s.setValue("bypass/byedpi", m_byedpiCheck->isChecked());
    s.setValue("bypass/byedpiHost", m_byedpiHostInput->text());
    s.setValue("bypass/geoBypass", m_geoBypassCheck->isChecked());
    s.setValue("bypass/geoCountry", m_geoCountryInput->text());
    s.setValue("bypass/forceIPv4", m_forceIPv4Check->isChecked());
    s.setValue("bypass/forceIPv6", m_forceIPv6Check->isChecked());
    s.setValue("bypass/legacySsl", m_legacySslCheck->isChecked());
    s.setValue("bypass/proxy", m_proxyInput->text());
}

void DownloadsView::onDownloadClicked() {
    QString url = m_urlInput->text().trimmed();
    if (url.isEmpty()) {
        m_statusLabel->setText("Please enter a valid URL.");
        m_statusLabel->setStyleSheet(QString("color: %1;").arg(StyleManager::error()));
        return;
    }
    
    m_statusLabel->setStyleSheet(QString("color: %1;").arg(StyleManager::textSecondary()));
    m_statusLabel->setText("Initializing download...");
    m_progressBar->setValue(0);
    m_progressBar->setVisible(true);
    
    m_downloadBtn->setEnabled(false);
    m_cancelBtn->setEnabled(true);
    m_urlInput->setEnabled(false);

    // Apply bypass settings to download manager
    m_downloadManager->setUseCookiesFromBrowser(m_cookiesCheck->isChecked());
    QString pc = m_playerClientCombo->currentText();
    m_downloadManager->setPlayerClient(pc == trL("dl_pc_default") ? "" : pc);
    m_downloadManager->setUseByeDpi(m_byedpiCheck->isChecked());
    m_downloadManager->setByeDpiHost(m_byedpiHostInput->text());
    m_downloadManager->setGeoBypass(m_geoBypassCheck->isChecked());
    m_downloadManager->setGeoBypassCountry(m_geoCountryInput->text());
    m_downloadManager->setForceIPv4(m_forceIPv4Check->isChecked());
    m_downloadManager->setForceIPv6(m_forceIPv6Check->isChecked());
    m_downloadManager->setLegacyServerConnect(m_legacySslCheck->isChecked());
    m_downloadManager->setProxy(m_proxyInput->text());

    m_downloadManager->startDownload(url);
}

void DownloadsView::onCancelClicked() {
    m_downloadManager->cancelDownload();
    
    m_downloadBtn->setEnabled(true);
    m_cancelBtn->setEnabled(false);
    m_urlInput->setEnabled(true);
    m_progressBar->setVisible(false);
}

void DownloadsView::onProgressUpdated(int percent) {
    m_progressBar->setValue(percent);
}

void DownloadsView::onStatusUpdated(const QString &text) {
    m_statusLabel->setText(text);
}

void DownloadsView::onDownloadCompleted(const QString &filePath) {
    m_lastDownloadedPath = filePath;
    // addTrack() already emits libraryUpdated() internally —
    // no need to emit it again here to avoid double UI refresh.
    m_libraryManager->addTrack(filePath);
    
    m_statusLabel->setText("Finished!");
    m_statusLabel->setStyleSheet(QString("color: %1;").arg(StyleManager::accent()));
    
    m_downloadBtn->setEnabled(true);
    m_cancelBtn->setEnabled(false);
    m_urlInput->setEnabled(true);
    m_urlInput->clear();
    m_progressBar->setVisible(false);
    
    // Ask user which playlist to add to
    showPlaylistSelector(filePath);
}

void DownloadsView::showPlaylistSelector(const QString &filePath) {
    QDialog dialog(this);
    dialog.setWindowTitle("Add to Playlist");
    dialog.setFixedSize(360, 300);
    dialog.setStyleSheet(QString(
        "QDialog { background-color: %1; }"
        "QLabel { color: %2; font-size: 14px; font-weight: bold; }"
        "QListWidget {"
        "   background-color: %3;"
        "   color: %4;"
        "   border: 1px solid %5;"
        "   border-radius: 8px;"
        "   padding: 4px;"
        "   font-size: 13px;"
        "}"
        "QListWidget::item {"
        "   padding: 8px 12px;"
        "   border-radius: 4px;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: %6;"
        "   color: %7;"
        "}"
        "QListWidget::item:hover {"
        "   background-color: %8;"
        "}"
        "QPushButton {"
        "   font-size: 13px;"
        "   font-weight: bold;"
        "   padding: 8px 16px;"
        "   border-radius: 6px;"
        "   border: none;"
        "}"
    ).arg(StyleManager::bgElevated(), StyleManager::textPrimary(),
          StyleManager::bgSurface(), StyleManager::textPrimary(), StyleManager::border(),
          StyleManager::accent(), "#FFFFFF", StyleManager::bgSurfaceHover()));
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);
    
    QLabel *label = new QLabel("Select playlist:", &dialog);
    layout->addWidget(label);
    
    QListWidget *listWidget = new QListWidget(&dialog);
    
    // Add "Don't add" option
    QListWidgetItem *skipItem = new QListWidgetItem("-- Don't add to playlist --");
    skipItem->setData(Qt::UserRole, -2);
    listWidget->addItem(skipItem);
    
    // Add "Favorites"
    QListWidgetItem *favItem = new QListWidgetItem(QString("\u2665 Favorites"));
    favItem->setData(Qt::UserRole, -1);
    listWidget->addItem(favItem);
    
    // Add existing playlists
    QList<QPair<int, QString>> playlists = m_libraryManager->getPlaylists();
    for (const auto &p : playlists) {
        QListWidgetItem *item = new QListWidgetItem(p.second);
        item->setData(Qt::UserRole, p.first);
        listWidget->addItem(item);
    }
    
    // Add "Create new" option
    QListWidgetItem *newItem = new QListWidgetItem("+ Create new playlist...");
    newItem->setData(Qt::UserRole, -3);
    listWidget->addItem(newItem);
    
    listWidget->setCurrentRow(0);
    layout->addWidget(listWidget);
    
    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    
    QPushButton *cancelBtn = new QPushButton("Cancel", &dialog);
    cancelBtn->setStyleSheet(QString("background-color: %1; color: %2;").arg(StyleManager::bgSurface(), StyleManager::textSecondary()));
    cancelBtn->setCursor(Qt::PointingHandCursor);
    
    QPushButton *okBtn = new QPushButton("Add", &dialog);
    okBtn->setStyleSheet(QString("background-color: %1; color: #FFFFFF;").arg(StyleManager::accent()));
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setEnabled(false);
    
    connect(listWidget, &QListWidget::currentItemChanged, this, [okBtn](QListWidgetItem *current, QListWidgetItem *) {
        okBtn->setEnabled(current != nullptr);
    });
    
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(okBtn);
    layout->addLayout(btnLayout);
    
    if (dialog.exec() == QDialog::Accepted) {
        QListWidgetItem *selected = listWidget->currentItem();
        if (!selected) return;
        
        int playlistId = selected->data(Qt::UserRole).toInt();
        
        if (playlistId == -3) {
            // Create new playlist
            bool ok;
            QString name = QInputDialog::getText(this, "New Playlist", "Enter playlist name:", QLineEdit::Normal, "", &ok);
            if (ok && !name.trimmed().isEmpty()) {
                m_libraryManager->createPlaylist(name.trimmed());
                // Find the new playlist ID (use the one with the highest ID to handle name duplicates)
                QList<QPair<int, QString>> playlists = m_libraryManager->getPlaylists();
                int newId = -1;
                for (const auto &p : playlists) {
                    if (p.second == name.trimmed() && p.first > newId) {
                        newId = p.first;
                    }
                }
                if (newId != -1) {
                    // addTrackToPlaylist() already emits libraryUpdated() internally
                    m_libraryManager->addTrackToPlaylist(newId, filePath);
                }
                m_statusLabel->setText(QString("Added to new playlist '%1'").arg(name.trimmed()));
                m_statusLabel->setStyleSheet(QString("color: %1;").arg(StyleManager::accent()));
            }
        } else if (playlistId == -1) {
            // Add to favorites
            m_libraryManager->toggleFavorite(filePath);
            m_statusLabel->setText("Added to Favorites");
            m_statusLabel->setStyleSheet(QString("color: %1;").arg(StyleManager::accent()));
        } else if (playlistId >= 0) {
            // Add to existing playlist
            // addTrackToPlaylist() already emits libraryUpdated() internally
            m_libraryManager->addTrackToPlaylist(playlistId, filePath);
            m_statusLabel->setText(QString("Added to '%1'").arg(selected->text()));
            m_statusLabel->setStyleSheet(QString("color: %1;").arg(StyleManager::accent()));
        }
    }
}

void DownloadsView::onDownloadFailed(const QString &error) {
    m_statusLabel->setText("Error: " + error);
    m_statusLabel->setStyleSheet(QString("color: %1;").arg(StyleManager::error()));
    
    m_downloadBtn->setEnabled(true);
    m_cancelBtn->setEnabled(false);
    m_urlInput->setEnabled(true);
    m_progressBar->setVisible(false);
}

void DownloadsView::applyQSS() {
    m_downloadBtn->setStyleSheet(StyleManager::primaryButtonStyle());
    m_cancelBtn->setStyleSheet(StyleManager::secondaryButtonStyle());
    m_progressBar->setStyleSheet(StyleManager::progressBarStyle());
}

void DownloadsView::refreshStyle() {
    applyQSS();

    // Re-apply label styles
    m_titleLabel->setStyleSheet(QString("font-size: 24px; font-weight: bold; color: %1;").arg(StyleManager::textPrimary()));
    m_subTitleLabel->setStyleSheet(QString("font-size: 13px; color: %1;").arg(StyleManager::textSecondary()));
    m_statusLabel->setStyleSheet(QString("font-size: 13px; color: %1;").arg(StyleManager::textSecondary()));
    m_bypassHeaderLabel->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1; margin-top: 12px;").arg(StyleManager::textPrimary()));

    // Style bypass checkboxes
    QString checkStyle = QString("color: %1; font-size: 13px; spacing: 8px;").arg(StyleManager::textPrimary());
    m_cookiesCheck->setStyleSheet(checkStyle);
    m_byedpiCheck->setStyleSheet(checkStyle);
    m_geoBypassCheck->setStyleSheet(checkStyle);
    m_forceIPv4Check->setStyleSheet(checkStyle);
    m_forceIPv6Check->setStyleSheet(checkStyle);
    m_legacySslCheck->setStyleSheet(checkStyle);

    // Style input fields
    QString inputStyle = QString(
        "QLineEdit { background-color: %1; color: %2; border: 1px solid %3; border-radius: 6px; padding: 4px 8px; font-size: 13px; }"
        "QLineEdit:disabled { background-color: %4; color: %5; }"
    ).arg(StyleManager::bgSurface(), StyleManager::textPrimary(), StyleManager::border(),
          StyleManager::bgElevated(), StyleManager::textMuted());
    m_byedpiHostInput->setStyleSheet(inputStyle);
    m_geoCountryInput->setStyleSheet(inputStyle);
    m_proxyInput->setStyleSheet(inputStyle);

    // Style combo box
    m_playerClientCombo->setStyleSheet(StyleManager::comboBoxStyle());
}
