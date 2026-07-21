#ifndef PLAYERBAR_H
#define PLAYERBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "MetadataManager.h"
#include "AudioPlayer.h"

class PlayerBar : public QWidget {
    Q_OBJECT
signals:
    void miniPlayerRequested();

public:
    explicit PlayerBar(QWidget *parent = nullptr);
    ~PlayerBar();

    void setAudioPlayer(AudioPlayer *player);
    void refreshStyle();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // Audio Player slots
    void updateTrackInfo(const TrackMetadata &track);
    void updatePlaybackState(QMediaPlayer::PlaybackState state);
    void updatePosition(qint64 pos);
    void updateDuration(qint64 dur);
    void updateVolume(int volume);
    void updateShuffle(bool enabled);
    void updateRepeat(AudioPlayer::RepeatMode mode);

    // UI actions
    void onPlayPauseClicked();
    void onSliderMoved(int position);
    void onVolumeSliderMoved(int value);
    void onShuffleClicked();
    void onRepeatClicked();
    void onMiniPlayerClicked();

private:
    AudioPlayer *m_audioPlayer = nullptr;
    qint64 m_currentDuration = 0;
    bool m_isSliderTracking = false;

    // UI Elements - Left (Track Info)
    QLabel *m_coverLabel;
    QLabel *m_titleLabel;
    QLabel *m_artistLabel;

    // UI Elements - Center (Controls)
    QPushButton *m_shuffleBtn;
    QPushButton *m_prevBtn;
    QPushButton *m_playPauseBtn;
    QPushButton *m_nextBtn;
    QPushButton *m_repeatBtn;
    QPushButton *m_likeBtn;
    
    QLabel *m_timeElapsedLabel;
    QSlider *m_progressSlider;
    QLabel *m_timeTotalLabel;

    // UI Elements - Right (Volume + MiniPlayer)
    QPushButton *m_volumeBtn;
    QSlider *m_volumeSlider;
    QPushButton *m_miniPlayerBtn;

    // Helpers
    QString formatTime(qint64 ms) const;
    void setupUI();
    void applyQSS();
};

#endif // PLAYERBAR_H
