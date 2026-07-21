#ifndef LIBRARYVIEW_H
#define LIBRARYVIEW_H

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QScrollArea>
#include <QLineEdit>
#include "LibraryManager.h"
#include "TrackList.h"

class LibraryView : public QWidget {
    Q_OBJECT
public:
    explicit LibraryView(LibraryManager *libMgr, QWidget *parent = nullptr);
    ~LibraryView();

    void refresh();
    void retranslateUI();
    void refreshStyle();
    
public slots:
    void openPlaylist(int id);
    void openArtist(const QString &artist);
    void setCurrentPlayingTrack(const QString &filePath);


signals:
    void playRequested(const TrackMetadata &track, const QList<TrackMetadata> &contextQueue, int playlistId = -1);
    void editRequested(const TrackMetadata &track);
    void downloadRequested(const TrackMetadata &track);

private slots:
    void onCreatePlaylistClicked();
    void onPlaylistRightClicked(int id, const QPoint &pos);
    void onBackToPlaylists();

private:
    LibraryManager *m_libraryManager;
    
    QStackedWidget *m_stackedWidget;
    
    QWidget *m_playlistsGridWidget;
    QScrollArea *m_playlistsScrollArea;
    QWidget *m_playlistsGridContainer;
    QGridLayout *m_playlistsLayout;
    
    TrackList *m_playlistDetailsList;
    
    void setupUI();
    void updatePlaylistsGrid();
    
    QLineEdit *m_searchBar;
    QString m_searchQuery;
    
    void onSearchTextChanged(const QString &text);
    void applyQSS();

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // LIBRARYVIEW_H
