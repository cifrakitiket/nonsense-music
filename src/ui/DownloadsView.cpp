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

DownloadsView::DownloadsView(DownloadManager *dlMgr, LibraryManager *libMgr, QWidget *parent)
    : QWidget(parent), m_downloadManager(dlMgr), m_libraryManager(libMgr) {
    setupUI();
    applyQSS();
    
    connect(m_downloadBtn, &QPushButton::clicked, this, &DownloadsView::onDownloadClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &DownloadsView::onCancelClicked);
    
    connect(m_downloadManager, &DownloadManager::progressUpdated, this, &DownloadsView::onProgressUpdated);
    connect(m_downloadManager, &DownloadManager::statusUpdated, this, &DownloadsView::onStatusUpdated);
    connect(m_downloadManager, &DownloadManager::downloadCompleted, this, &DownloadsView::onDownloadCompleted);
    connect(m_downloadManager, &DownloadManager::downloadFailed, this, &DownloadsView::onDownloadFailed);
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
}

void DownloadsView::retranslateUI() {
    m_titleLabel->setText(trL("dl_title"));
    m_subTitleLabel->setText(trL("dl_subtitle"));
    m_downloadBtn->setText(trL("btn_download"));
    m_cancelBtn->setText(trL("btn_cancel"));
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
}
