#include "LibraryView.h"
#include "TranslationManager.h"
#include "StyleManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QMouseEvent>
#include <QSqlQuery>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QLineEdit>
#include <QPushButton>
#include "IconProvider.h"

class PlaylistCard : public QWidget {
    Q_OBJECT
public:
    PlaylistCard(int id, const QString &name, const QString &coverPath, QWidget *parent = nullptr)
        : QWidget(parent), m_id(id), m_name(name), m_coverPath(coverPath) {

        setFixedSize(164, 210);
        setCursor(Qt::PointingHandCursor);

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(12, 12, 12, 12);
        layout->setSpacing(8);

        m_coverLabel = new QLabel(this);
        m_coverLabel->setFixedSize(140, 140);
        m_coverLabel->setAlignment(Qt::AlignCenter);
        m_coverLabel->setStyleSheet(
            QString("background-color: %1; border-radius: 4px;")
            .arg(StyleManager::bgSurface()));

        if (!coverPath.isEmpty() && QFile::exists(coverPath)) {
            QPixmap pm(coverPath);
            if (!pm.isNull()) {
                QPixmap scaled = pm.scaled(140, 140, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                QPixmap rounded(140, 140);
                rounded.fill(Qt::transparent);
                QPainter painter(&rounded);
                painter.setRenderHint(QPainter::Antialiasing);
                QPainterPath path;
                path.addRoundedRect(0, 0, 140, 140, 4, 4);
                painter.setClipPath(path);
                painter.drawPixmap((140-scaled.width())/2, (140-scaled.height())/2, scaled);
                m_coverLabel->setPixmap(rounded);
            } else {
                setDefaultCover();
            }
        } else {
            setDefaultCover();
        }

        m_titleLabel = new QLabel(name, this);
        m_titleLabel->setStyleSheet(
            QString("font-weight: bold; color: %1; font-size: 13px; background: transparent;")
            .arg(StyleManager::textPrimary()));
        m_titleLabel->setWordWrap(false);
        QFontMetrics fm(m_titleLabel->font());
        m_titleLabel->setText(fm.elidedText(name, Qt::ElideRight, 138));

        layout->addWidget(m_coverLabel);
        layout->addWidget(m_titleLabel);
        layout->addStretch();

        applyQSS();
    }
    
    int id() const { return m_id; }
    QString name() const { return m_name; }
    
signals:
    void clicked(int id);
    void rightClicked(int id, const QPoint &pos);

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            emit clicked(m_id);
        } else if (event->button() == Qt::RightButton) {
            emit rightClicked(m_id, event->globalPosition().toPoint());
        }
    }
    
private:
    int m_id;
    QString m_name;
    QString m_coverPath;
    QLabel *m_coverLabel;
    QLabel *m_titleLabel;
    
    void setDefaultCover() {
        if (m_id == -1) {
            m_coverLabel->setPixmap(IconProvider::getPixmap(IconType::Heart, QSize(48,48), QColor(StyleManager::accent())));
        } else {
            m_coverLabel->setPixmap(IconProvider::getPixmap(IconType::Playlist, QSize(48,48), QColor(StyleManager::textMuted())));
        }
    }
    
    void applyQSS() {
        setStyleSheet(
            QString(
            // Spotify card: #181818 bg, 8px radius, transition on hover
            "QWidget {"
            "   background-color: %1;"
            "   border-radius: 8px;"
            "}"
            "QWidget:hover {"
            "   background-color: %2;"
            "}"
            ).arg(StyleManager::bgElevated(), StyleManager::bgSurface())
        );
    }
};

LibraryView::LibraryView(LibraryManager *libMgr, QWidget *parent) 
    : QWidget(parent), m_libraryManager(libMgr) {
    setupUI();
    applyQSS();
    // Defer initial grid update so scroll area has correct width after layout
    QTimer::singleShot(0, this, &LibraryView::refresh);
    
    connect(m_libraryManager, &LibraryManager::libraryUpdated, this, &LibraryView::refresh);
    if(m_searchBar) {
        connect(m_searchBar, &QLineEdit::textChanged, this, &LibraryView::onSearchTextChanged);
    }
}

LibraryView::~LibraryView() {}

void LibraryView::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    m_stackedWidget = new QStackedWidget(this);
    
    m_playlistsGridWidget = new QWidget(this);
    QVBoxLayout *gridOuterLayout = new QVBoxLayout(m_playlistsGridWidget);
    gridOuterLayout->setContentsMargins(20, 16, 20, 12);
    gridOuterLayout->setSpacing(10);
    
    QWidget *actionsRow = new QWidget(m_playlistsGridWidget);
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsRow);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(12);
    
    m_searchBar = new QLineEdit(actionsRow);
    m_searchBar->setPlaceholderText(trL("search_placeholder"));
    m_searchBar->setFixedWidth(240);
    m_searchBar->setMinimumHeight(36);
    m_searchBar->setStyleSheet(StyleManager::inputStyle());
    
    QPushButton *createPlaylistBtn = new QPushButton(trL("playlist_new"), actionsRow);
    createPlaylistBtn->setMinimumHeight(34);
    createPlaylistBtn->setMaximumHeight(42);
    createPlaylistBtn->setCursor(Qt::PointingHandCursor);
    createPlaylistBtn->setStyleSheet(
        QString(
        "QPushButton {"
        "   background-color: %1;"
        "   color: %2;"
        "   border: 1px solid %3;"
        "   border-radius: 19px;"
        "   padding: 0 18px 0 18px;"
        "   font-size: 13px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: %4;"
        "   border-color: %5;"
        "}"
        ).arg(StyleManager::bgSurface(), StyleManager::textPrimary(), StyleManager::border(),
              StyleManager::bgSurfaceHover(), StyleManager::accent())
    );
    actionsLayout->addWidget(m_searchBar);
    actionsLayout->addStretch();
    actionsLayout->addWidget(createPlaylistBtn);
    
    gridOuterLayout->addWidget(actionsRow);
    
    m_playlistsScrollArea = new QScrollArea(m_playlistsGridWidget);
    m_playlistsScrollArea->setWidgetResizable(true);
    m_playlistsScrollArea->setFrameShape(QFrame::NoFrame);
    m_playlistsScrollArea->setStyleSheet("background-color: transparent;");

    m_playlistsGridContainer = new QWidget(m_playlistsScrollArea);
    m_playlistsGridContainer->setObjectName("PlaylistsGridContainer");
    m_playlistsGridContainer->setStyleSheet("QWidget#PlaylistsGridContainer { background-color: transparent; }");
    
    m_playlistsLayout = new QGridLayout(m_playlistsGridContainer);
    m_playlistsLayout->setContentsMargins(0, 0, 0, 0);
    m_playlistsLayout->setSpacing(4);
    
    m_playlistsGridContainer->setLayout(m_playlistsLayout);
    m_playlistsScrollArea->setWidget(m_playlistsGridContainer);
    
    gridOuterLayout->addWidget(m_playlistsScrollArea, 1);
    m_stackedWidget->addWidget(m_playlistsGridWidget);
    
    m_playlistDetailsList = new TrackList(m_libraryManager, TrackList::ViewMode::Playlist, this);
    m_stackedWidget->addWidget(m_playlistDetailsList);
    
    mainLayout->addWidget(m_stackedWidget, 1);
    
    connect(createPlaylistBtn, &QPushButton::clicked, this, &LibraryView::onCreatePlaylistClicked);
    connect(m_playlistDetailsList, &TrackList::playRequested, this, &LibraryView::playRequested);
    connect(m_playlistDetailsList, &TrackList::editRequested, this, &LibraryView::editRequested);
    connect(m_playlistDetailsList, &TrackList::downloadRequested, this, &LibraryView::downloadRequested);
    connect(m_playlistDetailsList, &TrackList::backToPlaylistsRequested, this, &LibraryView::onBackToPlaylists);
}

void LibraryView::onCreatePlaylistClicked() {
    bool ok;
    QString name = QInputDialog::getText(this, trL("playlist_new"), trL("playlist_name_prompt"), QLineEdit::Normal, "", &ok);
    if (ok && !name.trimmed().isEmpty()) {
        m_libraryManager->createPlaylist(name.trimmed());
        updatePlaylistsGrid();
    }
}

void LibraryView::openPlaylist(int id) {
    if (id == -1) {
        m_playlistDetailsList->setViewMode(TrackList::ViewMode::Favorites);
    } else {
        m_playlistDetailsList->setViewMode(TrackList::ViewMode::Playlist);
        m_playlistDetailsList->setPlaylistId(id);
    }
    m_stackedWidget->setCurrentIndex(1);
}

void LibraryView::openArtist(const QString &artist) {
    m_playlistDetailsList->setViewMode(TrackList::ViewMode::Artist);
    m_playlistDetailsList->setArtist(artist);
    m_stackedWidget->setCurrentIndex(1);
}

void LibraryView::setCurrentPlayingTrack(const QString &filePath) {
    m_playlistDetailsList->setCurrentPlayingTrack(filePath);
}

void LibraryView::onPlaylistRightClicked(int id, const QPoint &pos) {
    if (id == -1) return;
    
    QMenu menu(this);
    menu.setStyleSheet(StyleManager::contextMenuStyle());
    QAction *renameAct = menu.addAction(trL("playlist_rename"));
    QAction *coverAct = menu.addAction(trL("playlist_change_cover"));
    QAction *deleteAct = menu.addAction(trL("playlist_delete"));
    
    QAction *selected = menu.exec(pos);
    if (selected == renameAct) {
        bool ok;
        QSqlQuery query;
        query.prepare("SELECT name FROM playlists WHERE id = :id");
        query.bindValue(":id", id);
        QString currentName;
        if (query.exec() && query.next()) {
            currentName = query.value(0).toString();
        }
        
        QString name = QInputDialog::getText(this, trL("playlist_rename"), trL("playlist_name_prompt"), QLineEdit::Normal, currentName, &ok);
        if (ok && !name.trimmed().isEmpty()) {
            m_libraryManager->renamePlaylist(id, name.trimmed());
            updatePlaylistsGrid();
        }
    } else if (selected == coverAct) {
        QString file = QFileDialog::getOpenFileName(this, trL("playlist_change_cover"), QDir::homePath(), "Images (*.png *.jpg *.jpeg);;All Files (*)");
        if (!file.isEmpty()) {
            QByteArray hash = QCryptographicHash::hash(file.toUtf8(), QCryptographicHash::Md5);
            QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/covers/";
            QDir().mkpath(cacheDir);
            QString destPath = cacheDir + "playlist_" + QString::number(id) + "_" + hash.toHex() + ".jpg";
            if (QFile::exists(destPath)) {
                QFile::remove(destPath);
            }
            if (QFile::copy(file, destPath)) {
                m_libraryManager->updatePlaylistCover(id, destPath);
                updatePlaylistsGrid();
            }
        }
    } else if (selected == deleteAct) {
        auto res = QMessageBox::question(this, trL("playlist_delete"), QString("Are you sure you want to delete this playlist?"), QMessageBox::Yes | QMessageBox::No);
        if (res == QMessageBox::Yes) {
            m_libraryManager->deletePlaylist(id);
            updatePlaylistsGrid();
        }
    }
}

void LibraryView::onBackToPlaylists() {
    m_stackedWidget->setCurrentIndex(0);
    updatePlaylistsGrid();
}

void LibraryView::onSearchTextChanged(const QString &text) {
    m_searchQuery = text;
    updatePlaylistsGrid();
}

void LibraryView::updatePlaylistsGrid() {
    QLayoutItem *item;
    while ((item = m_playlistsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        if (item->layout()) {
            // we have no layouts in items, but just in case
        }
        delete item;
    }
    
    QString q = m_searchQuery.trimmed().toLower();
    int row = 0;
    int col = 0;
    int maxCols = qMax(1, (m_playlistsScrollArea->width() - 40) / 180); // 164 width + 16 spacing
    
    // Add Favorites if it matches search
    if (q.isEmpty() || trL("favorites_playlist").toLower().contains(q)) {
        PlaylistCard *favCard = new PlaylistCard(-1, trL("favorites_playlist"), "", m_playlistsGridContainer);
        connect(favCard, &PlaylistCard::clicked, this, &LibraryView::openPlaylist);
        connect(favCard, &PlaylistCard::rightClicked, this, &LibraryView::onPlaylistRightClicked);
        m_playlistsLayout->addWidget(favCard, row, col);
        col++;
        if (col >= maxCols) { col = 0; row++; }
    }
    
    QList<QPair<int, QString>> playlists = m_libraryManager->getPlaylists();
    
    // Default sort by name for grid
    std::sort(playlists.begin(), playlists.end(), [](const auto &a, const auto &b){
        return a.second.compare(b.second, Qt::CaseInsensitive) < 0;
    });

    for (const auto &p : playlists) {
        if (!q.isEmpty() && !p.second.toLower().contains(q)) continue;
        
        int id = p.first;
        QString title = p.second;
        QString cover = m_libraryManager->getPlaylistCover(id);
        
        PlaylistCard *card = new PlaylistCard(id, title, cover, m_playlistsGridContainer);
        connect(card, &PlaylistCard::clicked, this, &LibraryView::openPlaylist);
        connect(card, &PlaylistCard::rightClicked, this, &LibraryView::onPlaylistRightClicked);
        
        m_playlistsLayout->addWidget(card, row, col);
        col++;
        if (col >= maxCols) { col = 0; row++; }
    }
}

void LibraryView::refresh() {
    m_playlistDetailsList->refresh();
    if (m_stackedWidget->currentIndex() == 0) {
        updatePlaylistsGrid();
    }
}

void LibraryView::retranslateUI() {
    m_playlistDetailsList->retranslateUI();
    if (m_stackedWidget->currentIndex() == 0) {
        updatePlaylistsGrid();
    }
}

void LibraryView::applyQSS() {}

void LibraryView::refreshStyle() {
    applyQSS();
    updatePlaylistsGrid();
}

void LibraryView::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (m_stackedWidget->currentIndex() == 0) {
        updatePlaylistsGrid();
    }
}

#include "LibraryView.moc"
