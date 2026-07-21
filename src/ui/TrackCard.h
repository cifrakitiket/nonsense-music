#ifndef TRACKCARD_H
#define TRACKCARD_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "MetadataManager.h"
#include "LibraryManager.h"

class TrackCard : public QWidget {
    Q_OBJECT
public:
    explicit TrackCard(LibraryManager *libMgr, const TrackMetadata &track, QWidget *parent = nullptr);
    ~TrackCard();

    TrackMetadata track() const;
    void updateTrackData(const TrackMetadata &track);
    void setPlaylistContext(int playlistId);
    void setRowNumber(int n);
    void setFavorite(bool isFavorite);
    void setPlaying(bool isPlaying);
    void retranslateUI();
    void refreshStyle();

signals:
    void playRequested(const TrackMetadata &track);
    void editRequested(const TrackMetadata &track);
    void favoriteToggled(const TrackMetadata &track);
    void downloadRequested(const TrackMetadata &track);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    LibraryManager *m_libraryManager;
    TrackMetadata  m_track;
    int m_playlistContextId = -1;
    int m_rowNumber = 1;

    // Left: index / play toggle
    QWidget     *m_indexWidget;
    QLabel      *m_rowNumLabel;
    QPushButton *m_playBtn;
    QPushButton *m_likeBtn;
    QPushButton *m_moreBtn;
    
    bool m_isPlaying;
    bool m_isHovered;

    // Album art
    QLabel *m_coverLabel;

    // Title + artist
    QLabel *m_titleLabel;
    QLabel *m_artistLabel;

    // Album name
    QLabel *m_albumLabel;

    // Favorite toggle + duration + cloud
    QPushButton *m_favoriteBtn;
    QLabel      *m_durationLabel;
    QLabel      *m_cloudIcon;
    QPushButton *m_downloadBtn;

    void setupUI();
    void applyQSS();
    void loadCover();
    void updateFavoriteState();
    QString formatDuration(int seconds) const;
};

#endif // TRACKCARD_H
