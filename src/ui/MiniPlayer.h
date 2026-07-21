#ifndef MINIPLAYER_H
#define MINIPLAYER_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QPoint>
#include "MetadataManager.h"
#include "AudioPlayer.h"

class MiniPlayer : public QWidget {
    Q_OBJECT
public:
    explicit MiniPlayer(AudioPlayer *player, QWidget *parent = nullptr);
    ~MiniPlayer();

    void setMainWindow(QWidget *mainWin);
    void refreshStyle();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void updateTrackInfo(const TrackMetadata &track);
    void updatePlaybackState(QMediaPlayer::PlaybackState state);
    void updatePosition(qint64 pos);
    void updateDuration(qint64 dur);
    void updateShuffle(bool enabled);
    void updateRepeat(AudioPlayer::RepeatMode mode);

    void onPlayPauseClicked();
    void onCloseClicked();
    void onPinClicked();
    void onSliderMoved(int position);
    void onShuffleClicked();
    void onRepeatClicked();

private:
    AudioPlayer *m_audioPlayer;
    QWidget *m_mainWindow;
    QPoint m_dragPosition;
    qint64 m_currentDuration = 0;
    bool m_isSliderTracking = false;

    QLabel *m_coverLabel;
    QLabel *m_titleLabel;
    QLabel *m_artistLabel;

    QPushButton *m_pinBtn;
    QPushButton *m_closeBtn;

    QLabel *m_timeElapsedLabel;
    QSlider *m_progressSlider;
    QLabel *m_timeTotalLabel;

    QPushButton *m_shuffleBtn;
    QPushButton *m_prevBtn;
    QPushButton *m_playPauseBtn;
    QPushButton *m_nextBtn;
    QPushButton *m_repeatBtn;
    QPushButton *m_likeBtn;

    void setupUI();
    void applyQSS();
    QString formatTime(qint64 ms) const;
};

#endif // MINIPLAYER_H
