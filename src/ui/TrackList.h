#ifndef TRACKLIST_H
#define TRACKLIST_H

#include <QWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include "LibraryManager.h"
#include "TrackCard.h"

class TrackList : public QWidget {
    Q_OBJECT
public:
    enum class ViewMode {
        AllTracks,
        Favorites,
        Playlist,
        Artist
    };

    enum class SortMode {
        Default,
        Alphabetical,
        Recents
    };

    explicit TrackList(LibraryManager *libMgr, ViewMode mode = ViewMode::AllTracks, QWidget *parent = nullptr);
    ~TrackList();

    void refresh();
    void setViewMode(ViewMode mode);
    void setPlaylistId(int id);
    int  playlistId() const;
    void setArtist(const QString &artist);
    
    void setCurrentPlayingTrack(const QString &filePath);
    QString artist() const { return m_artistName; }
    void retranslateUI();
    void refreshStyle();

signals:
    void playRequested(const TrackMetadata &track, const QList<TrackMetadata> &contextQueue, int playlistId = -1);
    void downloadRequested(const TrackMetadata &track);
    void editRequested(const TrackMetadata &track);
    void backToPlaylistsRequested();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event)   override;
    void dropEvent(QDropEvent *event)           override;

private slots:
    void onSearchTextChanged(const QString &text);
    void onPlayCardRequested(const TrackMetadata &track);
    void onFavoriteToggled(const TrackMetadata &track);
    void onAddFilesClicked();

    void onRenamePlaylistClicked();
    void onChangePlaylistCoverClicked();
    void onDeletePlaylistClicked();
    void showSortMenu();

private:
    LibraryManager *m_libraryManager;
    ViewMode        m_viewMode;
    int             m_playlistId;
    QString         m_artistName;
    QString         m_searchQuery;
    QString         m_currentPlayingFilePath;
    SortMode        m_sortMode = SortMode::Default;

    QLineEdit   *m_searchBar;
    QPushButton *m_sortBtn;
    QPushButton *m_addFilesBtn;
    QScrollArea *m_scrollArea;
    QWidget     *m_listContainer;
    QVBoxLayout *m_listLayout;
    QWidget     *m_headerRow;      // Column-header bar (like Spotify's track table header)
    QList<TrackCard*> m_cards;

    QWidget     *m_playlistHeaderWidget;
    QLabel      *m_playlistCoverLabel;
    QLabel      *m_playlistNameLabel;
    QPushButton *m_renamePlaylistBtn;
    QPushButton *m_changeCoverBtn;
    QPushButton *m_deletePlaylistBtn;
    QPushButton *m_backToPlaylistsBtn;

    void setupUI();
    void buildHeaderRow();
    void updateList();
    void updatePlaylistHeader();
    QList<TrackMetadata> getFilteredTracks() const;
};

#endif // TRACKLIST_H
