#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QScrollArea>
#include <QLabel>
#include <QLineEdit>
#include "LibraryManager.h"
#include "IconProvider.h"

class Sidebar : public QWidget {
    Q_OBJECT
public:
    enum class FilterMode { Playlists, Tracks };
    enum class SortMode { Alphabetical, Recents };

    explicit Sidebar(LibraryManager *libMgr, QWidget *parent = nullptr);
    ~Sidebar();

    void setCurrentTab(int index);
    void selectPlaylist(int id);
    void clearTabSelection();
    void setCurrentPlayingPlaylist(int id);
    void retranslateUI();
    void refreshStyle();
    
    // Updates the playlists list
    void updatePlaylists();

signals:
    void tabChanged(int index);
    void playlistClicked(int id);
    void trackClicked(const TrackMetadata &track);
    void createPlaylistRequested();

private:
    LibraryManager *m_libraryManager;

    QButtonGroup *m_buttonGroup;
    QVBoxLayout  *m_layout;

    QPushButton *m_downloadsBtn;
    QPushButton *m_accountBtn;
    QPushButton *m_settingsBtn;

    // Library section
    FilterMode m_filterMode;
    SortMode m_sortMode;
    QString m_searchQuery;

    QLabel *m_yourLibraryLabel;
    QPushButton *m_createPlaylistBtn;
    QPushButton *m_filterPlaylistsBtn;
    QPushButton *m_filterTracksBtn;
    QPushButton *m_searchBtn;
    QLineEdit *m_searchBar;
    QPushButton *m_recentsBtn;
    
    QScrollArea *m_scrollArea;
    QWidget *m_scrollContainer;
    QVBoxLayout *m_scrollLayout;
    QVBoxLayout *m_playlistsLayout;

    int m_currentPlayingPlaylistId = -1;

    void setupUI();
    QPushButton *createNavButton(const QString &text, int id, IconType iconType);
    QString      navButtonStyle(bool checked) const;
    QWidget     *createSidebarItem(int id, const QString &title, const QString &subtitle, const QString &coverPath, bool isArtist = false);

private slots:
    void         showSortMenu();
};

#endif // SIDEBAR_H
