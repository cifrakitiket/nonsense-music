#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "MetadataManager.h"

class AudioPlayer : public QObject {
    Q_OBJECT
public:
    enum class RepeatMode {
        RepeatOff,
        RepeatOne,
        RepeatAll
    };
    Q_ENUM(RepeatMode)

    explicit AudioPlayer(QObject *parent = nullptr);
    ~AudioPlayer();

    // Playback control
    void setPlaylist(const QList<TrackMetadata> &tracks, int startIndex = 0);
    void playTrack(int index);
    void play();
    void pause();
    void stop();
    void next();
    void previous();
    
    // Seek and Volume
    void seek(qint64 positionMs);
    void setVolume(int volumePercent); // 0 to 100
    int volume() const;
    void toggleMute();
    bool isMuted() const;

    // Toggle Modes
    void setShuffle(bool enabled);
    bool isShuffle() const;
    void setRepeatMode(RepeatMode mode);
    RepeatMode repeatMode() const;

    // Current State Getters
    bool isPlaying() const;
    qint64 position() const;
    qint64 duration() const;
    int currentIndex() const;
    TrackMetadata currentTrack() const;
    QList<TrackMetadata> playlist() const;

signals:
    void trackChanged(const TrackMetadata &track);
    void playbackStateChanged(QMediaPlayer::PlaybackState state);
    void positionChanged(qint64 positionMs);
    void durationChanged(qint64 durationMs);
    void volumeChanged(int volumePercent);
    void playlistChanged();
    void shuffleChanged(bool shuffle);
    void repeatModeChanged(RepeatMode mode);

private slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPositionChanged(qint64 pos);
    void onDurationChanged(qint64 dur);
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);

private:
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    
    QList<TrackMetadata> m_playlist;
    QList<TrackMetadata> m_originalPlaylist; // To preserve track order for shuffle toggles
    int m_currentIndex = -1;
    
    int m_previousVolume = 100;
    bool m_isMuted = false;
    
    bool m_shuffle = false;
    RepeatMode m_repeatMode = RepeatMode::RepeatOff;
    
    void shufflePlaylist();
};

#endif // AUDIOPLAYER_H
