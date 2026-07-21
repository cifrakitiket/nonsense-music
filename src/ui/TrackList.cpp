#include "TrackList.h"
#include "TranslationManager.h"
#include "StyleManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QResizeEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileDialog>
#include <QStandardPaths>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>
#include <QPainter>
#include <QPainterPath>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include "IconProvider.h"

TrackList::TrackList(LibraryManager *libMgr, ViewMode mode, QWidget *parent)
    : QWidget(parent), m_libraryManager(libMgr), m_viewMode(mode), m_playlistId(-1)
{
    setAcceptDrops(true);
    setupUI();
    refresh();
    connect(m_libraryManager, &LibraryManager::libraryUpdated, this, &TrackList::refresh,
            Qt::QueuedConnection);
}

TrackList::~TrackList() {}

// ─────────────────────────────────────────────────────────────────────────────
// setupUI
// ─────────────────────────────────────────────────────────────────────────────
void TrackList::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 16, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->setAlignment(Qt::AlignTop);

    // ── Playlist / Favorites header ─────────────────────────────────────────
    m_playlistHeaderWidget = new QWidget(this);
    {
        QHBoxLayout *headerLayout = new QHBoxLayout(m_playlistHeaderWidget);
        headerLayout->setContentsMargins(24, 0, 24, 20);
        headerLayout->setSpacing(24);
        headerLayout->setAlignment(Qt::AlignTop);

        m_playlistCoverLabel = new QLabel(m_playlistHeaderWidget);
        m_playlistCoverLabel->setMinimumSize(160, 160);
        m_playlistCoverLabel->setMaximumSize(200, 200);
        m_playlistCoverLabel->setAlignment(Qt::AlignCenter);
        m_playlistCoverLabel->setStyleSheet(
            QString("background-color: %1; border-radius: 4px; font-size: 56px; color: %2;")
            .arg(StyleManager::bgSurface(), StyleManager::textMuted()));
        m_playlistCoverLabel->setStyleSheet(
            QString("background-color: %1; border-radius: 4px; font-size: 56px; color: %2;"
                    "box-shadow: 0 8px 24px rgba(0,0,0,0.5);")
            .arg(StyleManager::bgSurface(), StyleManager::textMuted()));

        QWidget *headerInfo = new QWidget(m_playlistHeaderWidget);
        QVBoxLayout *infoLayout = new QVBoxLayout(headerInfo);
        infoLayout->setContentsMargins(0, 0, 0, 0);
        infoLayout->setSpacing(8);
        infoLayout->setAlignment(Qt::AlignBottom);

        QLabel *typeLabel = new QLabel("PLAYLIST", headerInfo);
        typeLabel->setStyleSheet(
            QString("font-size: 11px; font-weight: bold; letter-spacing: 2px; color: %1;")
            .arg(StyleManager::textSecondary()));

        m_playlistNameLabel = new QLabel("Playlist Name", headerInfo);
        m_playlistNameLabel->setStyleSheet(
            QString("font-size: 48px; font-weight: bold; color: %1; letter-spacing: -1px;")
            .arg(StyleManager::textPrimary()));

        QWidget *headerButtons = new QWidget(headerInfo);
        QHBoxLayout *btnLayout = new QHBoxLayout(headerButtons);
        btnLayout->setContentsMargins(0, 12, 0, 0);
        btnLayout->setSpacing(12);

        m_renamePlaylistBtn  = new QPushButton(trL("playlist_rename"), headerButtons);
        m_changeCoverBtn     = new QPushButton(trL("playlist_change_cover"), headerButtons);
        m_deletePlaylistBtn  = new QPushButton(trL("playlist_delete"), headerButtons);
        m_backToPlaylistsBtn = new QPushButton(trL("playlist_back"), headerButtons);

        for (auto *b : { m_renamePlaylistBtn, m_changeCoverBtn, m_deletePlaylistBtn })
            b->setStyleSheet(StyleManager::subtleButtonStyle());
        m_backToPlaylistsBtn->setStyleSheet(StyleManager::accentButtonSmallStyle());

        for (auto *b : { m_renamePlaylistBtn, m_changeCoverBtn,
                         m_deletePlaylistBtn, m_backToPlaylistsBtn }) {
            b->setCursor(Qt::PointingHandCursor);
            b->setMinimumHeight(32);
        }

        btnLayout->addWidget(m_backToPlaylistsBtn);
        btnLayout->addWidget(m_renamePlaylistBtn);
        btnLayout->addWidget(m_changeCoverBtn);
        btnLayout->addWidget(m_deletePlaylistBtn);
        btnLayout->addStretch();

        infoLayout->addStretch();
        infoLayout->addWidget(typeLabel);
        infoLayout->addWidget(m_playlistNameLabel);
        infoLayout->addWidget(headerButtons);

        headerLayout->addWidget(m_playlistCoverLabel);
        headerLayout->addWidget(headerInfo, 1);
    }

    mainLayout->addWidget(m_playlistHeaderWidget);
    m_playlistHeaderWidget->setVisible(false);

    // ── Search + Add button row ─────────────────────────────────────────────
    QWidget *topRow = new QWidget(this);
    {
        QHBoxLayout *topLayout = new QHBoxLayout(topRow);
        topLayout->setContentsMargins(24, 0, 24, 12);
        topLayout->setSpacing(12);

        m_searchBar = new QLineEdit(topRow);
        m_searchBar->setPlaceholderText(trL("search_placeholder"));
        m_searchBar->setFixedWidth(240);
        m_searchBar->setMinimumHeight(36);
        m_searchBar->setStyleSheet(StyleManager::inputStyle());
        
        m_sortBtn = new QPushButton("Недавние \u2261", this);
        m_sortBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::List, QSize(16,16), QColor("#B3B3B3"))));
        m_sortBtn->setCursor(Qt::PointingHandCursor);
        m_sortBtn->setStyleSheet("QPushButton { background: transparent; color: #B3B3B3; font-size: 14px; font-weight: bold; border: none; padding-right: 4px; }"
                                 "QPushButton:hover { color: #FFFFFF; }");

        m_addFilesBtn = new QPushButton(trL("add_files"), this);
        m_addFilesBtn->setCursor(Qt::PointingHandCursor);
        m_addFilesBtn->setMinimumHeight(36);
        m_addFilesBtn->setMinimumWidth(180);
        m_addFilesBtn->setStyleSheet(StyleManager::primaryButtonStyle());

        topLayout->addWidget(m_searchBar);
        topLayout->addStretch();
        topLayout->addWidget(m_sortBtn);
        topLayout->addSpacing(16);
        topLayout->addWidget(m_addFilesBtn);
    }
    mainLayout->addWidget(topRow);

    // ── Column header row ──────────────────────────────────────────────────
    buildHeaderRow();
    mainLayout->addWidget(m_headerRow);

    // ── Scroll area with vertical track list ───────────────────────────────
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet(
        QString("QScrollArea { background-color: transparent; } %1")
        .arg(StyleManager::scrollbarStyle()));

    m_listContainer = new QWidget(m_scrollArea);
    m_listContainer->setObjectName("ListContainer");
    m_listContainer->setStyleSheet("QWidget#ListContainer { background-color: transparent; }");

    m_listLayout = new QVBoxLayout(m_listContainer);
    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setSpacing(0);

    m_listContainer->setLayout(m_listLayout);
    m_scrollArea->setWidget(m_listContainer);

    mainLayout->addWidget(m_scrollArea, 1);

    // ── Connections ─────────────────────────────────────────────────────────
    connect(m_searchBar,         &QLineEdit::textChanged,   this, &TrackList::onSearchTextChanged);
    connect(m_sortBtn,           &QPushButton::clicked,     this, &TrackList::showSortMenu);
    connect(m_addFilesBtn,       &QPushButton::clicked,     this, &TrackList::onAddFilesClicked);
    connect(m_backToPlaylistsBtn,&QPushButton::clicked,     this, &TrackList::backToPlaylistsRequested);
    connect(m_renamePlaylistBtn, &QPushButton::clicked,     this, &TrackList::onRenamePlaylistClicked);
    connect(m_changeCoverBtn,    &QPushButton::clicked,     this, &TrackList::onChangePlaylistCoverClicked);
    connect(m_deletePlaylistBtn, &QPushButton::clicked,     this, &TrackList::onDeletePlaylistClicked);
}

// ─────────────────────────────────────────────────────────────────────────────
// buildHeaderRow — Spotify-style column labels above the track list
// Columns must match TrackCard layout exactly (same padding / column widths)
// ─────────────────────────────────────────────────────────────────────────────
void TrackList::buildHeaderRow() {
    m_headerRow = new QWidget(this);
    m_headerRow->setFixedHeight(36);
    m_headerRow->setObjectName("TrackListHeader");
    m_headerRow->setStyleSheet(
        QString("QWidget#TrackListHeader { border-bottom: 1px solid %1; background: transparent; }")
        .arg(StyleManager::border()));

    QHBoxLayout *h = new QHBoxLayout(m_headerRow);
    h->setContentsMargins(16, 0, 16, 0);
    h->setSpacing(0);

    auto makeHdr = [&](const QString &text, int fixedW = -1, Qt::Alignment align = Qt::AlignLeft | Qt::AlignVCenter) {
        QLabel *lbl = new QLabel(text, m_headerRow);
        lbl->setAlignment(align);
        lbl->setStyleSheet(
            QString("color: %1; font-size: 11px; font-weight: bold; letter-spacing: 1px; background: transparent;")
            .arg(StyleManager::textMuted()));
        if (fixedW > 0) lbl->setFixedWidth(fixedW);
        return lbl;
    };
    
    auto makeIconHdr = [&](IconType icon, int fixedW = -1, Qt::Alignment align = Qt::AlignLeft | Qt::AlignVCenter) {
        QLabel *lbl = new QLabel(m_headerRow);
        lbl->setAlignment(align);
        lbl->setPixmap(IconProvider::getPixmap(icon, QSize(16,16), QColor(StyleManager::textMuted())));
        lbl->setStyleSheet("background: transparent;");
        if (fixedW > 0) lbl->setFixedWidth(fixedW);
        return lbl;
    };

    // #  (48 px — matches m_indexWidget)
    h->addWidget(makeHdr("#", 48, Qt::AlignCenter));
    h->addSpacing(12);
    // cover placeholder (40 px)
    h->addSpacing(40);
    h->addSpacing(12);
    // TITLE (stretch)
    h->addWidget(makeHdr("TITLE"), 1);
    h->addSpacing(16);
    // ALBUM (180 px)
    h->addWidget(makeHdr("ALBUM", 180));
    h->addSpacing(16);
    // ♡ placeholder (28 px)
    h->addSpacing(28);
    h->addSpacing(16);
    // ⏱ (50 px, right-aligned)
    h->addWidget(makeIconHdr(IconType::Clock, 50, Qt::AlignRight | Qt::AlignVCenter));
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────
void TrackList::setViewMode(ViewMode mode) {
    if (m_viewMode != mode) {
        m_viewMode = mode;
        refresh();
    }
}

void TrackList::setPlaylistId(int id) {
    m_playlistId = id;
    refresh();
}

int TrackList::playlistId() const { return m_playlistId; }


void TrackList::setCurrentPlayingTrack(const QString &filePath) {
    m_currentPlayingFilePath = filePath;
    for (TrackCard *card : m_cards) {
        card->setPlaying(card->track().filePath == filePath);
    }
}

void TrackList::setArtist(const QString &artist) {
    m_artistName = artist;
    refresh();
}

// ─────────────────────────────────────────────────────────────────────────────
// refresh — rebuild card list from DB
// ─────────────────────────────────────────────────────────────────────────────
void TrackList::refresh() {
    // Safely remove & delete previous cards
    for (TrackCard *card : m_cards) {
        m_listLayout->removeWidget(card);
        card->deleteLater();
    }
    m_cards.clear();

    updatePlaylistHeader();

    const QList<TrackMetadata> tracks = getFilteredTracks();
    for (const TrackMetadata &track : tracks) {
        TrackCard *card = new TrackCard(m_libraryManager, track, m_listContainer);
        if (m_viewMode == ViewMode::Playlist)
            card->setPlaylistContext(m_playlistId);

        connect(card, &TrackCard::playRequested,   this, &TrackList::onPlayCardRequested);
        connect(card, &TrackCard::editRequested,   this, &TrackList::editRequested);
        connect(card, &TrackCard::favoriteToggled, this, [this](const TrackMetadata &t) {
            m_libraryManager->toggleFavorite(t.filePath);
            refresh();
        });
        connect(card, &TrackCard::downloadRequested, this, &TrackList::downloadRequested);
        
        if (track.filePath == m_currentPlayingFilePath) {
            card->setPlaying(true);
        }
        
        m_cards.append(card);
    }

    updateList();
}

// ─────────────────────────────────────────────────────────────────────────────
// updateList — place cards in the vertical layout
// ─────────────────────────────────────────────────────────────────────────────
void TrackList::updateList() {
    // Remove layout items without deleting widgets (they live in m_cards)
    while (m_listLayout->count() > 0) {
        QLayoutItem *item = m_listLayout->takeAt(0);
        delete item;
    }

    for (int i = 0; i < m_cards.size(); ++i) {
        m_cards[i]->setRowNumber(i + 1);
        m_listLayout->addWidget(m_cards[i]);
    }
    m_listLayout->addStretch();
}

// ─────────────────────────────────────────────────────────────────────────────
// Slot implementations
// ─────────────────────────────────────────────────────────────────────────────
void TrackList::onSearchTextChanged(const QString &text) {
    m_searchQuery = text;
    refresh();
}

void TrackList::onPlayCardRequested(const TrackMetadata &track) {
    QList<TrackMetadata> queue;
    for (TrackCard *card : m_cards)
        queue.append(card->track());
    emit playRequested(track, queue);
}

void TrackList::onFavoriteToggled(const TrackMetadata &track) {
    m_libraryManager->toggleFavorite(track.filePath);
}

QList<TrackMetadata> TrackList::getFilteredTracks() const {
    QList<TrackMetadata> tracks;
    if (m_viewMode == ViewMode::Playlist)
        tracks = m_libraryManager->getPlaylistTracks(m_playlistId);
    else if (m_viewMode == ViewMode::Favorites)
        tracks = m_libraryManager->getFavoriteTracks();
    else if (m_viewMode == ViewMode::Artist)
        tracks = m_libraryManager->getArtistTracks(m_artistName);
    else
        tracks = m_libraryManager->getAllTracks();

    if (!m_searchQuery.trimmed().isEmpty()) {
        QList<TrackMetadata> filtered;
        for (const auto &t : tracks)
            if (t.title.contains(m_searchQuery, Qt::CaseInsensitive)  ||
                t.artist.contains(m_searchQuery, Qt::CaseInsensitive) ||
                t.album.contains(m_searchQuery, Qt::CaseInsensitive))
                filtered.append(t);
        tracks = filtered;
    }
    
    if (m_sortMode == SortMode::Alphabetical) {
        std::sort(tracks.begin(), tracks.end(), [](const TrackMetadata &a, const TrackMetadata &b){
            return a.title.compare(b.title, Qt::CaseInsensitive) < 0;
        });
    } else if (m_sortMode == SortMode::Recents) {
        std::reverse(tracks.begin(), tracks.end());
    }
    
    return tracks;
}

// ─────────────────────────────────────────────────────────────────────────────
// Playlist header update
// ─────────────────────────────────────────────────────────────────────────────
void TrackList::updatePlaylistHeader() {
    if (m_viewMode == ViewMode::Playlist || m_viewMode == ViewMode::Favorites || m_viewMode == ViewMode::Artist) {
        m_playlistHeaderWidget->setVisible(true);
        m_addFilesBtn->setVisible(true);
        m_headerRow->setVisible(true);

        if (m_viewMode == ViewMode::Favorites) {
            m_playlistNameLabel->setText(trL("favorites_playlist"));
            m_playlistCoverLabel->setText("");
            m_playlistCoverLabel->setPixmap(IconProvider::getPixmap(IconType::Heart, QSize(80,80), QColor(StyleManager::accent())));
            m_playlistCoverLabel->setAlignment(Qt::AlignCenter);
            m_playlistCoverLabel->setStyleSheet(
                QString("background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
                        "stop:0 #5C0A0A, stop:1 #2D0505);"
                        "border-radius: 4px;"));

            m_renamePlaylistBtn->setVisible(false);
            m_changeCoverBtn->setVisible(false);
            m_deletePlaylistBtn->setVisible(false);
            m_backToPlaylistsBtn->setVisible(true);
        } else if (m_viewMode == ViewMode::Artist) {
            m_playlistNameLabel->setText(m_artistName.isEmpty() ? trL("unknown_artist") : m_artistName);
            m_playlistCoverLabel->setText("");
            m_playlistCoverLabel->setPixmap(IconProvider::getPixmap(IconType::Note, QSize(80,80), QColor(StyleManager::textMuted())));
            m_playlistCoverLabel->setAlignment(Qt::AlignCenter);
            m_playlistCoverLabel->setStyleSheet(
                QString("background-color: %1; border-radius: 100px;")
                .arg(StyleManager::bgElevated()));

            m_renamePlaylistBtn->setVisible(false);
            m_changeCoverBtn->setVisible(false);
            m_deletePlaylistBtn->setVisible(false);
            m_backToPlaylistsBtn->setVisible(false); // Sidebar handles back navigation for artists
        } else {
            // Regular playlist
            QSqlQuery query;
            query.prepare("SELECT name FROM playlists WHERE id = :id");
            query.bindValue(":id", m_playlistId);
            if (query.exec() && query.next())
                m_playlistNameLabel->setText(query.value(0).toString());

            // Cover
            QString coverPath = m_libraryManager->getPlaylistCover(m_playlistId);
            if (!coverPath.isEmpty() && QFile::exists(coverPath)) {
                QPixmap pm(coverPath);
                if (!pm.isNull()) {
                    QPixmap scaled = pm.scaled(180, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    QPixmap rounded(180, 180);
                    rounded.fill(Qt::transparent);
                    QPainter painter(&rounded);
                    painter.setRenderHint(QPainter::Antialiasing);
                    QPainterPath path;
                    path.addRoundedRect(0, 0, 180, 180, 4, 4);
                    painter.setClipPath(path);
                    painter.drawPixmap((180 - scaled.width()) / 2, (180 - scaled.height()) / 2, scaled);
                    m_playlistCoverLabel->setPixmap(rounded);
                } else {
                    m_playlistCoverLabel->setText(QString("\u266B"));
                    m_playlistCoverLabel->setStyleSheet(
                        QString("background-color: %1; border-radius: 4px; font-size: 56px; color: %2;")
                        .arg(StyleManager::bgSurface(), StyleManager::textMuted()));
                }
            } else {
                m_playlistCoverLabel->setText(QString("\u266B"));
                m_playlistCoverLabel->setStyleSheet(
                    QString("background-color: %1; border-radius: 4px; font-size: 56px; color: %2;")
                    .arg(StyleManager::bgSurface(), StyleManager::textMuted()));
            }

            m_renamePlaylistBtn->setVisible(true);
            m_changeCoverBtn->setVisible(true);
            m_deletePlaylistBtn->setVisible(true);
            m_backToPlaylistsBtn->setVisible(true);
        }
    } else {
        m_playlistHeaderWidget->setVisible(false);
        m_addFilesBtn->setVisible(true);
        m_headerRow->setVisible(true);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Playlist management slots
// ─────────────────────────────────────────────────────────────────────────────
void TrackList::onRenamePlaylistClicked() {
    bool ok;
    QString name = QInputDialog::getText(
        this, trL("playlist_rename"), trL("playlist_name_prompt"),
        QLineEdit::Normal, m_playlistNameLabel->text(), &ok);
    if (ok && !name.trimmed().isEmpty()) {
        m_libraryManager->renamePlaylist(m_playlistId, name.trimmed());
        refresh();
    }
}

void TrackList::onChangePlaylistCoverClicked() {
    QString file = QFileDialog::getOpenFileName(
        this, trL("playlist_change_cover"), QDir::homePath(),
        "Images (*.png *.jpg *.jpeg);;All Files (*)");
    if (!file.isEmpty()) {
        QByteArray hash = QCryptographicHash::hash(file.toUtf8(), QCryptographicHash::Md5);
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/covers/";
        QDir().mkpath(cacheDir);
        QString dest = cacheDir + "playlist_" + QString::number(m_playlistId) + "_" + hash.toHex() + ".jpg";
        if (QFile::exists(dest)) QFile::remove(dest);
        if (QFile::copy(file, dest)) {
            m_libraryManager->updatePlaylistCover(m_playlistId, dest);
            refresh();
        }
    }
}

void TrackList::onDeletePlaylistClicked() {
    auto res = QMessageBox::question(
        this, trL("playlist_delete"),
        "Are you sure you want to delete this playlist?",
        QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::Yes) {
        m_libraryManager->deletePlaylist(m_playlistId);
        emit backToPlaylistsRequested();
    }
}

void TrackList::showSortMenu() {
    QMenu menu(this);
    menu.setStyleSheet("QMenu { background-color: #282828; color: #FFFFFF; border: 1px solid #3E3E3E; }"
                       "QMenu::item { padding: 4px 24px; }"
                       "QMenu::item:selected { background-color: #3E3E3E; }");
    QAction *defaultAct = menu.addAction("По умолчанию");
    QAction *alphaAct = menu.addAction("По алфавиту");
    QAction *recentAct = menu.addAction("Недавние");
    
    QAction *res = menu.exec(m_sortBtn->mapToGlobal(QPoint(0, m_sortBtn->height())));
    if (res == alphaAct) {
        m_sortMode = SortMode::Alphabetical;
        m_sortBtn->setText("По алфавиту \u2261");
        refresh();
    } else if (res == recentAct) {
        m_sortMode = SortMode::Recents;
        m_sortBtn->setText("Недавние \u2261");
        refresh();
    } else if (res == defaultAct) {
        m_sortMode = SortMode::Default;
        m_sortBtn->setText("По умолчанию \u2261");
        refresh();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// retranslateUI / refreshStyle
// ─────────────────────────────────────────────────────────────────────────────
void TrackList::retranslateUI() {
    m_searchBar->setPlaceholderText(trL("search_placeholder"));
    m_addFilesBtn->setText(trL("add_files"));
    m_backToPlaylistsBtn->setText(trL("playlist_back"));
    m_renamePlaylistBtn->setText(trL("playlist_rename"));
    m_changeCoverBtn->setText(trL("playlist_change_cover"));
    m_deletePlaylistBtn->setText(trL("playlist_delete"));
    for (TrackCard *card : m_cards) card->retranslateUI();
}

void TrackList::refreshStyle() {
    QString bgColor = StyleManager::isDynamicBg() ? "transparent" : StyleManager::bgPrimary();
    setStyleSheet(QString("QWidget#TrackListWidget { background-color: %1; }").arg(bgColor));
    
    m_playlistCoverLabel->setStyleSheet(
        QString("background-color: %1; border-radius: 4px; font-size: 56px; color: %2;")
        .arg(StyleManager::bgSurface(), StyleManager::textMuted()));
    m_playlistNameLabel->setStyleSheet(
        QString("font-size: 48px; font-weight: bold; color: %1; letter-spacing: -1px; background: transparent;")
        .arg(StyleManager::textPrimary()));
    m_renamePlaylistBtn->setStyleSheet(StyleManager::subtleButtonStyle());
    m_changeCoverBtn->setStyleSheet(StyleManager::subtleButtonStyle());
    m_deletePlaylistBtn->setStyleSheet(StyleManager::subtleButtonStyle());
    m_backToPlaylistsBtn->setStyleSheet(StyleManager::accentButtonSmallStyle());
    m_searchBar->setStyleSheet(StyleManager::inputStyle());
    m_addFilesBtn->setStyleSheet(StyleManager::primaryButtonStyle());
    m_scrollArea->setStyleSheet(
        QString("QScrollArea { background-color: transparent; } %1")
        .arg(StyleManager::scrollbarStyle()));
    m_headerRow->setStyleSheet(
        QString("QWidget#TrackListHeader { border-bottom: 1px solid %1; background: transparent; }")
        .arg(StyleManager::border()));
    for (TrackCard *card : m_cards) card->refreshStyle();
}

// ─────────────────────────────────────────────────────────────────────────────
// Drag & Drop
// ─────────────────────────────────────────────────────────────────────────────
void TrackList::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) event->acceptProposedAction();
}

void TrackList::dragMoveEvent(QDragMoveEvent *event) {
    if (event->mimeData()->hasUrls()) event->acceptProposedAction();
}

void TrackList::dropEvent(QDropEvent *event) {
    for (const QUrl &url : event->mimeData()->urls()) {
        QString path = url.toLocalFile();
        if (path.isEmpty()) continue;
        QString ext = QFileInfo(path).suffix().toLower();
        if (ext == "mp3" || ext == "flac" || ext == "wav" || ext == "ogg") {
            m_libraryManager->addTrack(path);
            if (m_viewMode == ViewMode::Playlist && m_playlistId != -1)
                m_libraryManager->addTrackToPlaylist(m_playlistId, path);
            else if (m_viewMode == ViewMode::Favorites)
                m_libraryManager->toggleFavorite(path);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Add files button
// ─────────────────────────────────────────────────────────────────────────────
void TrackList::onAddFilesClicked() {
    QString musicPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    if (musicPath.isEmpty()) musicPath = QDir::homePath();

    QStringList files = QFileDialog::getOpenFileNames(
        this, "Select Audio Files to Add", musicPath,
        "Audio Files (*.mp3 *.flac *.wav *.ogg);;All Files (*)");

    for (const QString &file : files) {
        m_libraryManager->addTrack(file);
        if (m_viewMode == ViewMode::Playlist && m_playlistId != -1)
            m_libraryManager->addTrackToPlaylist(m_playlistId, file);
        else if (m_viewMode == ViewMode::Favorites)
            m_libraryManager->toggleFavorite(file);
    }
}
