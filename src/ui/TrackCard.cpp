#include "TrackCard.h"
#include "TranslationManager.h"
#include "StyleManager.h"
#include <QPainter>
#include <QPainterPath>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QFile>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include "IconProvider.h"

TrackCard::TrackCard(LibraryManager *libMgr, const TrackMetadata &track, QWidget *parent)
    : QWidget(parent), m_libraryManager(libMgr), m_track(track),
      m_playlistContextId(-1), m_rowNumber(1), m_isPlaying(false), m_isHovered(false)
{
    setupUI();
    applyQSS();
    loadCover();
    updateFavoriteState();
}

TrackCard::~TrackCard() {}

TrackMetadata TrackCard::track() const { return m_track; }

void TrackCard::setPlaylistContext(int playlistId) {
    m_playlistContextId = playlistId;
}

void TrackCard::setRowNumber(int n) {
    m_rowNumber = n;
    if (!m_isPlaying && !m_isHovered) {
        m_rowNumLabel->setText(QString::number(n));
    }
}

void TrackCard::setPlaying(bool isPlaying) {
    if (m_isPlaying != isPlaying) {
        m_isPlaying = isPlaying;
        refreshStyle();
    }
}

void TrackCard::retranslateUI() {}

// ─────────────────────────────────────────────────────────────────────────────
// setupUI — Spotify-style horizontal track row
// Layout: [48 index] [12] [40 cover] [12] [title+artist flex] [16] [180 album]
//         [16] [28 fav] [16] [50 duration] [16]
// ─────────────────────────────────────────────────────────────────────────────
void TrackCard::setupUI() {
    setFixedHeight(56);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setObjectName("TrackRow");
    setCursor(Qt::PointingHandCursor);

    QHBoxLayout *row = new QHBoxLayout(this);
    row->setContentsMargins(16, 0, 16, 0);
    row->setSpacing(0);

    // ── Column 1: index / ▶ (48 px fixed) ───────────────────────────────
    m_indexWidget = new QWidget(this);
    m_indexWidget->setFixedSize(48, 56);
    m_indexWidget->setStyleSheet("background: transparent;");

    m_rowNumLabel = new QLabel(QString::number(m_rowNumber), m_indexWidget);
    m_rowNumLabel->setGeometry(0, 0, 48, 56);
    m_rowNumLabel->setAlignment(Qt::AlignCenter);
    m_rowNumLabel->setStyleSheet(
        QString("color: %1; font-size: 14px; background: transparent;")
        .arg(StyleManager::textSecondary()));

    m_playBtn = new QPushButton(m_indexWidget);
    m_playBtn->setObjectName("RowPlayButton");
    m_playBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::Play, QSize(16,16), QColor(StyleManager::textPrimary()))));
    m_playBtn->setIconSize(QSize(16,16));
    m_playBtn->setGeometry(8, 12, 32, 32);
    m_playBtn->setCursor(Qt::PointingHandCursor);
    m_playBtn->hide();

    row->addWidget(m_indexWidget);
    row->addSpacing(12);

    // ── Column 2: cover (40×40) ───────────────────────────────────────────
    m_coverLabel = new QLabel(this);
    m_coverLabel->setFixedSize(40, 40);
    m_coverLabel->setAlignment(Qt::AlignCenter);
    m_coverLabel->setStyleSheet(
        QString("background-color: %1; border-radius: 4px;")
        .arg(StyleManager::bgSurface()));
    m_coverLabel->setPixmap(IconProvider::getPixmap(IconType::Note, QSize(24,24), QColor(StyleManager::textMuted())));

    row->addWidget(m_coverLabel);
    row->addSpacing(12);

    // ── Column 3: title + artist (stretch) ────────────────────────────────
    QWidget *textBox = new QWidget(this);
    textBox->setStyleSheet("background: transparent;");
    textBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    textBox->setFixedHeight(56);

    QVBoxLayout *textLayout = new QVBoxLayout(textBox);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(2);
    textLayout->setAlignment(Qt::AlignVCenter);

    m_titleLabel = new QLabel(m_track.title.isEmpty() ? "Unknown Title" : m_track.title, textBox);
    m_titleLabel->setStyleSheet(
        QString("color: %1; font-size: 14px; font-weight: 500; background: transparent;")
        .arg(StyleManager::textPrimary()));

    m_artistLabel = new QLabel(m_track.artist.isEmpty() ? "Unknown Artist" : m_track.artist, textBox);
    m_artistLabel->setStyleSheet(
        QString("color: %1; font-size: 12px; background: transparent;")
        .arg(StyleManager::textSecondary()));

    textLayout->addWidget(m_titleLabel);
    textLayout->addWidget(m_artistLabel);

    row->addWidget(textBox, 1);
    row->addSpacing(16);

    // ── Column 4: album (180 px) ──────────────────────────────────────────
    m_albumLabel = new QLabel(this);
    m_albumLabel->setFixedWidth(180);
    m_albumLabel->setStyleSheet(
        QString("color: %1; font-size: 13px; background: transparent;")
        .arg(StyleManager::textSecondary()));
    {
        QFontMetrics fm(m_albumLabel->font());
        m_albumLabel->setText(fm.elidedText(
            m_track.album.isEmpty() ? "" : m_track.album, Qt::ElideRight, 170));
    }

    row->addWidget(m_albumLabel);
    row->addSpacing(16);

    // ── Column 5: favourite (28 px) ───────────────────────────────────────
    m_favoriteBtn = new QPushButton(this);
    m_favoriteBtn->setObjectName("FavoriteBtn");
    m_favoriteBtn->setFixedSize(28, 28);
    m_favoriteBtn->setCursor(Qt::PointingHandCursor);
    
    row->addWidget(m_favoriteBtn);
    row->addSpacing(16);

    // ── Column 5.5: cloud / download ──────────────────────────────────────
    m_cloudIcon = new QLabel(this);
    m_cloudIcon->setPixmap(IconProvider::getPixmap(IconType::Account, QSize(16,16), QColor(StyleManager::accent())));
    m_cloudIcon->setStyleSheet("background: transparent;");
    m_cloudIcon->setFixedWidth(20);
    
    m_downloadBtn = new QPushButton(this);
    m_downloadBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::Download, QSize(16,16), QColor(StyleManager::textSecondary()))));
    m_downloadBtn->setIconSize(QSize(16,16));
    m_downloadBtn->setFixedSize(28, 28);
    m_downloadBtn->setCursor(Qt::PointingHandCursor);
    m_downloadBtn->setStyleSheet(QString(
        "QPushButton { background: transparent; color: %1; border-radius: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: %2; }"
    ).arg(StyleManager::textSecondary(), StyleManager::bgSurfaceHover()));
    
    row->addWidget(m_cloudIcon);
    row->addWidget(m_downloadBtn);
    row->addSpacing(8);

    // ── Column 6: duration (50 px) ────────────────────────────────────────
    m_durationLabel = new QLabel(formatDuration(m_track.duration), this);
    m_durationLabel->setFixedWidth(50);
    m_durationLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_durationLabel->setStyleSheet(
        QString("color: %1; font-size: 13px; background: transparent;")
        .arg(StyleManager::textSecondary()));

    row->addWidget(m_durationLabel);

    // ── Connections ────────────────────────────────────────────────────────
    connect(m_playBtn,    &QPushButton::clicked, this, [this]() {
        if (m_track.isCloudOnly) {
        } else {
            emit playRequested(m_track);
        }
    });
    connect(m_favoriteBtn,&QPushButton::clicked, this, [this]() { emit favoriteToggled(m_track); });
    connect(m_downloadBtn,&QPushButton::clicked, this, [this]() { emit downloadRequested(m_track); });
    
    m_cloudIcon->setVisible(m_track.isCloudOnly);
    m_downloadBtn->setVisible(m_track.isCloudOnly);
}

// ─────────────────────────────────────────────────────────────────────────────
void TrackCard::updateTrackData(const TrackMetadata &track) {
    m_track = track;
    m_titleLabel->setText(track.title.isEmpty() ? "Unknown Title" : track.title);
    m_artistLabel->setText(track.artist.isEmpty() ? "Unknown Artist" : track.artist);
    {
        QFontMetrics fm(m_albumLabel->font());
        m_albumLabel->setText(fm.elidedText(track.album, Qt::ElideRight, 170));
    }
    m_durationLabel->setText(formatDuration(track.duration));
    m_cloudIcon->setVisible(track.isCloudOnly);
    m_downloadBtn->setVisible(track.isCloudOnly);
    loadCover();
    updateFavoriteState();
}

void TrackCard::loadCover() {
    QString coverPath = m_track.coverMimeType;
    if (coverPath.isEmpty() || !QFile::exists(coverPath)) {
        QByteArray hash = QCryptographicHash::hash(
            m_track.filePath.toUtf8(), QCryptographicHash::Md5);
        QString cacheDir =
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/covers/";
        coverPath = cacheDir + hash.toHex() + ".jpg";
    }

    if (QFile::exists(coverPath)) {
        QPixmap pm(coverPath);
        if (!pm.isNull()) {
            QPixmap scaled = pm.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QPixmap rounded(40, 40);
            rounded.fill(Qt::transparent);
            QPainter painter(&rounded);
            painter.setRenderHint(QPainter::Antialiasing);
            QPainterPath path;
            path.addRoundedRect(0, 0, 40, 40, 4, 4);
            painter.setClipPath(path);
            painter.drawPixmap((40 - scaled.width()) / 2, (40 - scaled.height()) / 2, scaled);
            m_coverLabel->setPixmap(rounded);
            return;
        }
    }
    m_coverLabel->setText(QString("\u266B"));
}

void TrackCard::updateFavoriteState() {
    bool isFav = m_libraryManager->isFavorite(m_track.filePath);
    if (isFav) {
        m_favoriteBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::Heart, QSize(16,16), QColor(StyleManager::accent()))));
        m_favoriteBtn->setStyleSheet(
            QString("background: transparent; border: none;")
        );
        m_favoriteBtn->show();
    } else {
        m_favoriteBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::HeartOutline, QSize(16,16), QColor(StyleManager::textSecondary()))));
        if (m_isHovered) {
            m_favoriteBtn->setStyleSheet(
                QString("background: transparent; border: none;")
            );
            m_favoriteBtn->show();
        } else {
            m_favoriteBtn->hide();
        }
    }
}

QString TrackCard::formatDuration(int seconds) const {
    if (seconds <= 0) return "--:--";
    return QString("%1:%2").arg(seconds / 60).arg(seconds % 60, 2, 10, QChar('0'));
}

// ─────────────────────────────────────────────────────────────────────────────
// Hover: show play button, dim row, reveal favorite
// ─────────────────────────────────────────────────────────────────────────────
void TrackCard::enterEvent(QEnterEvent *event) {
    Q_UNUSED(event);
    setStyleSheet("QWidget#TrackRow { background-color: " + QString(StyleManager::bgSurface()) + "; }");
    m_isHovered = true;
    m_playBtn->show();
    m_rowNumLabel->hide();
    updateFavoriteState();
    setStyleSheet(
        QString("QWidget#TrackRow { background-color: %1; border-radius: 4px; }")
        .arg(StyleManager::bgElevated()));
    QWidget::enterEvent(event);
}

void TrackCard::leaveEvent(QEvent *event) {
    m_isHovered = false;
    m_playBtn->hide();
    m_rowNumLabel->show();
    updateFavoriteState();
    setStyleSheet("QWidget#TrackRow { background-color: transparent; border-radius: 4px; }");
    QWidget::leaveEvent(event);
}

void TrackCard::mouseDoubleClickEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    emit playRequested(m_track);
}

// ─────────────────────────────────────────────────────────────────────────────
// Context menu — identical logic, just keep context menu items
// ─────────────────────────────────────────────────────────────────────────────
void TrackCard::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);
    menu.setStyleSheet(StyleManager::contextMenuStyle());

    auto lang = TranslationManager::instance().currentLanguage();
    using L = TranslationManager::Language;

    QString playText =
        (lang == L::Russian || lang == L::RussianPreReform) ? "Воспроизвести" :
        (lang == L::Ukrainian) ? "Відтворити" : "Play";

    QString favText =
        (lang == L::Russian)          ? "В избранное / Убрать" :
        (lang == L::Ukrainian)        ? "В обране / Прибрати"  :
        (lang == L::RussianPreReform) ? "Въ избранное / Убрать" : "Toggle Favorite";

    QAction *playAct = menu.addAction(playText);
    QAction *favAct  = menu.addAction(favText);
    QAction *editAct = menu.addAction(trL("edit_info"));
    menu.addSeparator();

    QAction *removePlaylistAct = nullptr;
    QMenu   *subMenu           = nullptr;
    QMap<QAction*, int> playlistMap;
    QAction *newPlaylistAct    = nullptr;

    if (m_playlistContextId != -1) {
        removePlaylistAct = menu.addAction(trL("remove_from_playlist"));
    } else {
        subMenu = menu.addMenu(trL("add_to_playlist"));
        subMenu->setStyleSheet(StyleManager::contextMenuStyle());
        for (const auto &p : m_libraryManager->getPlaylists()) {
            QAction *act = subMenu->addAction(p.second);
            playlistMap[act] = p.first;
        }
        subMenu->addSeparator();
        newPlaylistAct = subMenu->addAction(trL("playlist_new"));
    }

    menu.addSeparator();
    QAction *deleteAct = menu.addAction(trL("delete_track"));

    QAction *sel = menu.exec(event->globalPos());
    if (!sel) return;

    if (sel == playAct) {
        emit playRequested(m_track);
    } else if (sel == favAct) {
        emit favoriteToggled(m_track);
    } else if (sel == editAct) {
        emit editRequested(m_track);
    } else if (sel == removePlaylistAct) {
        m_libraryManager->removeTrackFromPlaylist(m_playlistContextId, m_track.filePath);
    } else if (sel == deleteAct) {
        auto res = QMessageBox::question(
            this, trL("delete_confirm_title"), trL("delete_confirm_text"),
            QMessageBox::Yes | QMessageBox::No);
        if (res == QMessageBox::Yes)
            m_libraryManager->deleteTrack(m_track.filePath);
    } else if (sel == newPlaylistAct) {
        bool ok;
        QString name = QInputDialog::getText(
            this, trL("playlist_new"), trL("playlist_name_prompt"),
            QLineEdit::Normal, "", &ok);
        if (ok && !name.trimmed().isEmpty()) {
            m_libraryManager->createPlaylist(name.trimmed());
            int newId = -1;
            for (const auto &p : m_libraryManager->getPlaylists())
                if (p.second == name.trimmed() && p.first > newId)
                    newId = p.first;
            if (newId != -1)
                m_libraryManager->addTrackToPlaylist(newId, m_track.filePath);
        }
    } else if (playlistMap.contains(sel)) {
        m_libraryManager->addTrackToPlaylist(playlistMap[sel], m_track.filePath);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void TrackCard::applyQSS() {
    setStyleSheet(
        QString(
        "QWidget#TrackRow { background-color: transparent; border-radius: 4px; }"
        "QPushButton#RowPlayButton {"
        "   background: transparent; border: none;"
        "}"
        )
    );
}

void TrackCard::refreshStyle() {
    applyQSS();
    m_coverLabel->setStyleSheet(
        QString("background-color: %1; border-radius: 4px;")
        .arg(StyleManager::bgSurface()));
        
    m_titleLabel->setStyleSheet(
        QString("color: %1; font-size: 14px; font-weight: 500; background: transparent;")
        .arg(m_isPlaying ? StyleManager::accent() : StyleManager::textPrimary()));
        
    m_artistLabel->setStyleSheet(
        QString("color: %1; font-size: 12px; background: transparent;")
        .arg(StyleManager::textSecondary()));
        
    m_albumLabel->setStyleSheet(
        QString("color: %1; font-size: 13px; background: transparent;")
        .arg(StyleManager::textSecondary()));
        
    m_durationLabel->setStyleSheet(
        QString("color: %1; font-size: 13px; background: transparent;")
        .arg(StyleManager::textSecondary()));
        
    if (m_isPlaying) {
        m_rowNumLabel->setText("");
        m_rowNumLabel->setPixmap(IconProvider::getPixmap(IconType::VolumeOn, QSize(16,16), QColor(StyleManager::accent())));
        m_rowNumLabel->setStyleSheet("background: transparent;");
    } else {
        m_rowNumLabel->setPixmap(QPixmap());
        m_rowNumLabel->setText(QString::number(m_rowNumber));
        m_rowNumLabel->setStyleSheet(
            QString("color: %1; font-size: 14px; background: transparent;")
            .arg(StyleManager::textSecondary()));
    }
        
    updateFavoriteState();
}
