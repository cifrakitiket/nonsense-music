#include "Sidebar.h"
#include "TranslationManager.h"
#include "StyleManager.h"
#include <QLabel>
#include <QFrame>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QFile>
#include <QLineEdit>
#include <QMenu>
#include <QAction>
#include "IconProvider.h"

// Helper widget for a playlist or artist item in the sidebar
class SidebarPlaylistItem : public QWidget {
    Q_OBJECT
public:
    SidebarPlaylistItem(int id, const QString &title, const QString &subtitle, const QString &coverPath, bool isArtist = false, QWidget *parent = nullptr)
        : QWidget(parent), m_id(id), m_isArtist(isArtist), m_artistName(isArtist ? title : ""), m_isTrack(false) {
        
        setFixedHeight(64);
        setCursor(Qt::PointingHandCursor);
        
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(12, 8, 12, 8);
        layout->setSpacing(12);
        
        QLabel *coverLabel = new QLabel(this);
        coverLabel->setFixedSize(48, 48);
        coverLabel->setAlignment(Qt::AlignCenter);
        
        if (id == -1 && !isArtist) {
            // Favorites
            coverLabel->setStyleSheet(QString("background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #450af5, stop:1 #8e8ee5); border-radius: 4px;"));
            QPixmap pm = IconProvider::getPixmap(IconType::Heart, QSize(24, 24), Qt::white);
            coverLabel->setPixmap(pm);
        } else {
            int radius = isArtist ? 24 : 4; // Circle for artists, rounded rect for playlists
            coverLabel->setStyleSheet(QString("background-color: %1; border-radius: %2px; color: %3; font-size: 20px;")
                                      .arg(StyleManager::bgElevated()).arg(radius).arg(StyleManager::textMuted()));
            
            if (!coverPath.isEmpty() && QFile::exists(coverPath)) {
                QPixmap pm(coverPath);
                if (!pm.isNull()) {
                    QPixmap scaled = pm.scaled(48, 48, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                    QPixmap rounded(48, 48);
                    rounded.fill(Qt::transparent);
                    QPainter painter(&rounded);
                    painter.setRenderHint(QPainter::Antialiasing);
                    QPainterPath path;
                    path.addRoundedRect(0, 0, 48, 48, radius, radius);
                    painter.setClipPath(path);
                    painter.drawPixmap((48 - scaled.width())/2, (48 - scaled.height())/2, scaled);
                    coverLabel->setPixmap(rounded);
                }
            } else {
                coverLabel->setPixmap(IconProvider::getPixmap(IconType::Note, QSize(24,24), QColor(StyleManager::textMuted())));
            }
        }
        
        QVBoxLayout *textLayout = new QVBoxLayout();
        textLayout->setContentsMargins(0, 2, 0, 2);
        textLayout->setSpacing(2);
        
        QLabel *titleLabel = new QLabel(title, this);
        titleLabel->setStyleSheet(QString("font-weight: 500; font-size: 14px; color: %1; background: transparent;").arg(StyleManager::textPrimary()));
        
        QLabel *subtitleLabel = new QLabel(subtitle, this);
        subtitleLabel->setStyleSheet(QString("font-size: 12px; color: %1; background: transparent;").arg(StyleManager::textSecondary()));
        
        textLayout->addWidget(titleLabel);
        textLayout->addWidget(subtitleLabel);
        textLayout->addStretch();
        
        layout->addWidget(coverLabel);
        layout->addLayout(textLayout);
        layout->addStretch();
        
        setStyleSheet(QString(
            "SidebarPlaylistItem { background: transparent; border-radius: 6px; }"
        ).arg(StyleManager::bgSurfaceHover()));
    }

    SidebarPlaylistItem(const TrackMetadata &track, QWidget *parent = nullptr)
        : QWidget(parent), m_id(-2), m_isArtist(false), m_isTrack(true), m_track(track) {
        
        setFixedHeight(64);
        setCursor(Qt::PointingHandCursor);
        
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(12, 8, 12, 8);
        layout->setSpacing(12);
        
        QLabel *coverLabel = new QLabel(this);
        coverLabel->setFixedSize(48, 48);
        coverLabel->setAlignment(Qt::AlignCenter);
        coverLabel->setStyleSheet(QString("background-color: %1; border-radius: 4px; color: %2; font-size: 20px;")
                                  .arg(StyleManager::bgElevated()).arg(StyleManager::textMuted()));
        
        if (!track.coverData.isNull()) {
            QPixmap pm;
            if (pm.loadFromData(track.coverData)) {
                QPixmap scaled = pm.scaled(48, 48, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                QPixmap rounded(48, 48);
                rounded.fill(Qt::transparent);
                QPainter painter(&rounded);
                painter.setRenderHint(QPainter::Antialiasing);
                QPainterPath path;
                path.addRoundedRect(0, 0, 48, 48, 4, 4);
                painter.setClipPath(path);
                painter.drawPixmap((48 - scaled.width())/2, (48 - scaled.height())/2, scaled);
                coverLabel->setPixmap(rounded);
            }
        } else {
            coverLabel->setPixmap(IconProvider::getPixmap(IconType::Note, QSize(24,24), QColor(StyleManager::textMuted())));
        }
        
        QVBoxLayout *textLayout = new QVBoxLayout();
        textLayout->setContentsMargins(0, 2, 0, 2);
        textLayout->setSpacing(2);
        
        QLabel *titleLabel = new QLabel(track.title.isEmpty() ? "Unknown Title" : track.title, this);
        titleLabel->setStyleSheet(QString("font-weight: 500; font-size: 14px; color: %1; background: transparent;").arg(StyleManager::textPrimary()));
        
        QLabel *subtitleLabel = new QLabel(track.artist.isEmpty() ? "Unknown Artist" : track.artist, this);
        subtitleLabel->setStyleSheet(QString("font-size: 12px; color: %1; background: transparent;").arg(StyleManager::textSecondary()));
        
        textLayout->addWidget(titleLabel);
        textLayout->addWidget(subtitleLabel);
        textLayout->addStretch();
        
        layout->addWidget(coverLabel);
        layout->addLayout(textLayout);
        layout->addStretch();
        
        setStyleSheet(QString(
            "SidebarPlaylistItem { background: transparent; border-radius: 6px; }"
            "SidebarPlaylistItem:hover { background-color: %1; }"
        ).arg(StyleManager::bgSurfaceHover()));
    }

    void setPlaying(bool isPlaying) {
        if (isPlaying) {
            setStyleSheet(QString(
                "SidebarPlaylistItem { background-color: %1; border-radius: 6px; }"
                "SidebarPlaylistItem:hover { background-color: %1; }"
            ).arg(StyleManager::bgSurfaceHover()));
            // Title highlight
            QList<QLabel*> labels = findChildren<QLabel*>();
            if (labels.size() > 1) {
                labels[1]->setStyleSheet(QString("font-weight: bold; font-size: 14px; color: %1; background: transparent;").arg(StyleManager::accent()));
            }
        } else {
            setStyleSheet(QString(
                "SidebarPlaylistItem { background: transparent; border-radius: 6px; }"
                "SidebarPlaylistItem:hover { background-color: %1; }"
            ).arg(StyleManager::bgSurfaceHover()));
            QList<QLabel*> labels = findChildren<QLabel*>();
            if (labels.size() > 1) {
                labels[1]->setStyleSheet(QString("font-weight: 500; font-size: 14px; color: %1; background: transparent;").arg(StyleManager::textPrimary()));
            }
        }
    }

signals:
    void clickedPlaylist(int id);
    void clickedArtist(const QString &artistName);
    void clickedTrack(const TrackMetadata &track);

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            if (m_isArtist) {
                emit clickedArtist(m_artistName);
            } else if (m_isTrack) {
                emit clickedTrack(m_track);
            } else {
                emit clickedPlaylist(m_id);
            }
        }
    }
    
private:
    int m_id;
    bool m_isArtist;
    QString m_artistName;
    bool m_isTrack;
    TrackMetadata m_track;
};

#include "Sidebar.moc"

Sidebar::Sidebar(LibraryManager *libMgr, QWidget *parent) 
    : QWidget(parent), m_libraryManager(libMgr), m_filterMode(FilterMode::Playlists), m_sortMode(SortMode::Alphabetical) {
    setObjectName("SpotifySidebar");
    setFixedWidth(280);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    // ── Logo ──────────────────────────────────────────────────────────────
    QWidget *logoWidget = new QWidget(this);
    logoWidget->setStyleSheet("background: transparent;");
    QHBoxLayout *logoLayout = new QHBoxLayout(logoWidget);
    logoLayout->setContentsMargins(20, 24, 20, 20);
    logoLayout->setSpacing(10);

    QLabel *logoIcon = new QLabel(this);
    QPixmap logoPix(":/logo.png");
    if (!logoPix.isNull()) {
        logoIcon->setPixmap(logoPix.scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    logoIcon->setFixedSize(36, 36);
    logoIcon->setAlignment(Qt::AlignCenter);
    logoIcon->setStyleSheet("background: transparent;");

    QLabel *logoLabel = new QLabel("Nonsense Music", this);
    logoLabel->setObjectName("SidebarLogo");
    logoLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #FFFFFF;"
        "letter-spacing: 0.3px; background: transparent;");

    logoLayout->addWidget(logoIcon);
    logoLayout->addWidget(logoLabel);
    logoLayout->addStretch();
    m_layout->addWidget(logoWidget);

    // ── Nav buttons (Bottom, prepared here) ────────────────────────────────
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);

    m_downloadsBtn = createNavButton(trL("sidebar_downloads"), 1, IconType::Download);
    m_accountBtn = createNavButton(trL("sidebar_account"), 2, IconType::Account);
    m_settingsBtn = createNavButton(trL("sidebar_settings"), 3, IconType::Settings);

    // ── Your Library Header ───────────────────────────────────────────────
    QWidget *libHeader = new QWidget(this);
    libHeader->setStyleSheet("background: transparent;");
    QHBoxLayout *libHeaderLayout = new QHBoxLayout(libHeader);
    libHeaderLayout->setContentsMargins(20, 8, 20, 8);
    
    m_yourLibraryLabel = new QLabel(trL("sidebar_your_library"), this);
    m_yourLibraryLabel->setStyleSheet("font-weight: bold; color: #B3B3B3; font-size: 14px;");
    
    m_createPlaylistBtn = new QPushButton(this);
    m_createPlaylistBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::Plus, QSize(24,24), QColor("#B3B3B3"))));
    m_createPlaylistBtn->setIconSize(QSize(20,20));
    m_createPlaylistBtn->setFixedSize(24, 24);
    m_createPlaylistBtn->setCursor(Qt::PointingHandCursor);
    m_createPlaylistBtn->setStyleSheet(
        "QPushButton { color: #B3B3B3; background: transparent; border: none; font-size: 18px; }"
        "QPushButton:hover { color: #FFFFFF; }"
    );
    connect(m_createPlaylistBtn, &QPushButton::clicked, this, &Sidebar::createPlaylistRequested);
    
    libHeaderLayout->addWidget(m_yourLibraryLabel);
    libHeaderLayout->addStretch();
    libHeaderLayout->addWidget(m_createPlaylistBtn);
    m_layout->addWidget(libHeader);
    
    // ── Search & Sort Row ─────────────────────────────────────────────────
    QWidget *sortRow = new QWidget(this);
    sortRow->setStyleSheet("background: transparent;");
    QHBoxLayout *sortLayout = new QHBoxLayout(sortRow);
    sortLayout->setContentsMargins(20, 0, 20, 4);
    
    m_searchBtn = new QPushButton(this);
    m_searchBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::Search, QSize(24,24), QColor("#B3B3B3"))));
    m_searchBtn->setIconSize(QSize(18,18));
    m_searchBtn->setFixedSize(24, 24);
    m_searchBtn->setStyleSheet("background: transparent; border: none;");
    m_searchBtn->setCursor(Qt::PointingHandCursor);
    
    m_searchBar = new QLineEdit(this);
    m_searchBar->setPlaceholderText("Search...");
    m_searchBar->setStyleSheet("QLineEdit { background: #282828; color: #FFFFFF; border: none; border-radius: 4px; padding: 2px 6px; }");
    m_searchBar->setVisible(false);
    
    connect(m_searchBtn, &QPushButton::clicked, this, [this]() {
        bool isVisible = m_searchBar->isVisible();
        m_searchBar->setVisible(!isVisible);
        if (!isVisible) m_searchBar->setFocus();
        else {
            m_searchBar->clear();
            m_searchQuery = "";
            updatePlaylists();
        }
    });
    
    connect(m_searchBar, &QLineEdit::textChanged, this, [this](const QString &text) {
        m_searchQuery = text;
        updatePlaylists();
    });
    
    m_recentsBtn = new QPushButton(trL("sidebar_recents") + " \u2261", this);
    m_recentsBtn->setStyleSheet("background: transparent; color: #B3B3B3; font-size: 12px; font-weight: 500; border: none; text-align: right;");
    m_recentsBtn->setCursor(Qt::PointingHandCursor);
    
    connect(m_recentsBtn, &QPushButton::clicked, this, &Sidebar::showSortMenu);
    
    sortLayout->addWidget(m_searchBtn);
    sortLayout->addWidget(m_searchBar);
    sortLayout->addStretch();
    sortLayout->addWidget(m_recentsBtn);
    m_layout->addWidget(sortRow);

    // ── Scroll Area for Playlists ─────────────────────────────────────────
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical { width: 8px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #5A5A5A; border-radius: 4px; }"
        "QScrollBar::handle:vertical:hover { background: #B3B3B3; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
    );
    
    m_scrollContainer = new QWidget(m_scrollArea);
    m_scrollContainer->setStyleSheet("background: transparent;");
    m_scrollLayout = new QVBoxLayout(m_scrollContainer);
    m_scrollLayout->setContentsMargins(8, 0, 8, 20);
    m_scrollLayout->setSpacing(0);
    
    m_scrollArea->setWidget(m_scrollContainer);
    m_layout->addWidget(m_scrollArea, 1);

    // ── Divider ───────────────────────────────────────────────────────────
    QFrame *divider = new QFrame(this);
    divider->setFrameShape(QFrame::HLine);
    divider->setFixedHeight(1);
    divider->setStyleSheet("background-color: #282828; margin: 12px 20px;");
    m_layout->addWidget(divider);

    // ── Bottom Nav buttons ────────────────────────────────────────────────
    m_layout->addWidget(m_accountBtn);
    m_layout->addWidget(m_downloadsBtn);
    m_layout->addWidget(m_settingsBtn);

    connect(m_buttonGroup, &QButtonGroup::idClicked, this, &Sidebar::tabChanged);
    
    if (m_libraryManager) {
        connect(m_libraryManager, &LibraryManager::libraryUpdated, this, &Sidebar::updatePlaylists);
        updatePlaylists();
    }

    setCurrentTab(0);
}

Sidebar::~Sidebar() {}

QPushButton *Sidebar::createNavButton(const QString &text, int id, IconType iconType) {
    QPushButton *btn = new QPushButton(text, this);
    btn->setIcon(QIcon(IconProvider::getPixmap(iconType, QSize(24,24), QColor("#B3B3B3"))));
    btn->setIconSize(QSize(20,20));
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(48);
    btn->setStyleSheet(navButtonStyle(false));
    m_buttonGroup->addButton(btn, id);
    return btn;
}

QString Sidebar::navButtonStyle(bool /*unused*/) const {
    return QString(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: #B3B3B3;"
        "   border: none;"
        "   border-left: 3px solid transparent;"
        "   border-radius: 0;"
        "   padding: 0 21px;"
        "   text-align: left;"
        "   font-size: 14px;"
        "   font-weight: 700;"
        "}"
        "QPushButton:hover {"
        "   color: #FFFFFF;"
        "}"
        "QPushButton:checked {"
        "   color: #FFFFFF;"
        "   border-left: 3px solid %1;"
        "   background-color: #282828;"
        "}"
    ).arg(StyleManager::accent());
}

void Sidebar::setCurrentTab(int index) {
    QAbstractButton *btn = m_buttonGroup->button(index);
    if (btn) btn->setChecked(true);
}

void Sidebar::selectPlaylist(int id) {
    clearTabSelection();
    // Usually we highlight the row, but right now the track list handles it.
    // If we wanted to keep the playlist selected, we'd do it here.
}

void Sidebar::setCurrentPlayingPlaylist(int id) {
    m_currentPlayingPlaylistId = id;
    updatePlaylists();
}

void Sidebar::clearTabSelection() {
    m_buttonGroup->setExclusive(false);
    for (QAbstractButton *btn : m_buttonGroup->buttons()) {
        btn->setChecked(false);
    }
    m_buttonGroup->setExclusive(true);
    refreshStyle();
}

void Sidebar::showSortMenu() {
    QMenu menu(this);
    menu.setStyleSheet(StyleManager::contextMenuStyle());
    
    QAction *alphaAct = menu.addAction("A-Z");
    QAction *recentAct = menu.addAction(trL("sidebar_recents"));
    
    if (m_sortMode == SortMode::Alphabetical) alphaAct->setCheckable(true), alphaAct->setChecked(true);
    if (m_sortMode == SortMode::Recents) recentAct->setCheckable(true), recentAct->setChecked(true);
    
    QAction *selected = menu.exec(m_recentsBtn->mapToGlobal(QPoint(0, m_recentsBtn->height())));
    if (selected == alphaAct) {
        m_sortMode = SortMode::Alphabetical;
        m_recentsBtn->setText("A-Z \u2261");
        updatePlaylists();
    } else if (selected == recentAct) {
        m_sortMode = SortMode::Recents;
        m_recentsBtn->setText(trL("sidebar_recents") + " \u2261");
        updatePlaylists();
    }
}

void Sidebar::retranslateUI() {
    m_downloadsBtn->setText(trL("sidebar_downloads"));
    m_accountBtn->setText(trL("sidebar_account"));
    m_settingsBtn->setText(trL("sidebar_settings"));
    
    m_yourLibraryLabel->setText(trL("sidebar_your_library"));
    if (m_sortMode == SortMode::Recents) {
        m_recentsBtn->setText(trL("sidebar_recents") + " \u2261");
    }
    
    updatePlaylists();
}

void Sidebar::refreshStyle() {
    setStyleSheet("QWidget#SpotifySidebar { background-color: " + StyleManager::getDynamicBg(StyleManager::bgSidebar(), 180) + "; }");
    for (QAbstractButton *btn : m_buttonGroup->buttons())
        btn->setStyleSheet(navButtonStyle(btn->isChecked()));
}

void Sidebar::updatePlaylists() {
    if (!m_libraryManager) return;
    
    QLayoutItem *child;
    while ((child = m_scrollLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
    
    QString q = m_searchQuery.trimmed().toLower();
    
    // 1. Favorites
    if (q.isEmpty() || trL("favorites_playlist").toLower().contains(q)) {
        SidebarPlaylistItem *favItem = new SidebarPlaylistItem(-1, trL("favorites_playlist"), "Плейлист \u2022 NonsenseMusic", "");
        connect(favItem, &SidebarPlaylistItem::clickedPlaylist, this, &Sidebar::playlistClicked);
        favItem->setPlaying(-1 == m_currentPlayingPlaylistId);
        m_scrollLayout->addWidget(favItem);
    }
    
    // 2. User Playlists
    QList<QPair<int, QString>> playlists = m_libraryManager->getPlaylists();
    
    if (m_sortMode == SortMode::Alphabetical) {
        std::sort(playlists.begin(), playlists.end(), [](const auto &a, const auto &b){
            return a.second.compare(b.second, Qt::CaseInsensitive) < 0;
        });
    } else if (m_sortMode == SortMode::Recents) {
        std::reverse(playlists.begin(), playlists.end());
    }
    
    for (const auto &p : playlists) {
        if (!q.isEmpty() && !p.second.toLower().contains(q)) continue;
        
        int id = p.first;
        QString title = p.second;
        QString cover = m_libraryManager->getPlaylistCover(id);
        
        SidebarPlaylistItem *item = new SidebarPlaylistItem(id, title, "Плейлист", cover);
        connect(item, &SidebarPlaylistItem::clickedPlaylist, this, &Sidebar::playlistClicked);
        item->setPlaying(id == m_currentPlayingPlaylistId);
        m_scrollLayout->addWidget(item);
    }
    
    m_scrollLayout->addStretch();
}
