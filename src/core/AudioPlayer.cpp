#include "AudioPlayer.h"
#include <QRandomGenerator>
#include <QUrl>
#include <QFileInfo>
#include <QDebug>

AudioPlayer::AudioPlayer(QObject *parent) : QObject(parent) {
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    
    // Connect player signals
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &AudioPlayer::onMediaStatusChanged);
    connect(m_player, &QMediaPlayer::positionChanged, this, &AudioPlayer::onPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &AudioPlayer::onDurationChanged);
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, &AudioPlayer::onPlaybackStateChanged);
    
    // Set default volume
    m_audioOutput->setVolume(1.0); // 100%
}

AudioPlayer::~AudioPlayer() {
    stop();
}

void AudioPlayer::setPlaylist(const QList<TrackMetadata> &tracks, int startIndex) {
    m_originalPlaylist = tracks;
    m_playlist = tracks;
    
    if (m_shuffle) {
        shufflePlaylist();
        // Find where our startIndex track went
        if (startIndex >= 0 && startIndex < tracks.size()) {
            QString targetPath = tracks[startIndex].filePath;
            for (int i = 0; i < m_playlist.size(); ++i) {
                if (m_playlist[i].filePath == targetPath) {
                    m_currentIndex = i;
                    break;
                }
            }
        } else {
            m_currentIndex = 0;
        }
    } else {
        m_currentIndex = (startIndex >= 0 && startIndex < m_playlist.size()) ? startIndex : 0;
    }
    
    emit playlistChanged();
    if (!m_playlist.isEmpty()) {
        playTrack(m_currentIndex);
    }
}

void AudioPlayer::playTrack(int index) {
    if (index < 0 || index >= m_playlist.size()) {
        stop();
        return;
    }
    
    m_currentIndex = index;
    TrackMetadata track = m_playlist[m_currentIndex];
    
    m_player->setSource(QUrl::fromLocalFile(track.filePath));
    m_player->play();
    
    emit trackChanged(track);
}

void AudioPlayer::play() {
    if (m_player->source().isEmpty() && !m_playlist.isEmpty()) {
        playTrack(0);
    } else {
        m_player->play();
    }
}

void AudioPlayer::pause() {
    m_player->pause();
}

void AudioPlayer::stop() {
    m_player->stop();
}

void AudioPlayer::next() {
    if (m_playlist.isEmpty()) return;
    
    int nextIndex = m_currentIndex + 1;
    if (nextIndex >= m_playlist.size()) {
        nextIndex = 0;
        
        if (m_shuffle) {
            // Re-shuffle the playlist so we get a new order
            QString lastPlayedFile = m_playlist[m_currentIndex].filePath;
            shufflePlaylist();
            
            // Prevent playing the same track twice in a row if there are multiple tracks
            if (m_playlist.size() > 1 && m_playlist[0].filePath == lastPlayedFile) {
                // Swap the first track with a random other track
                int swapIndex = QRandomGenerator::global()->bounded(1, m_playlist.size());
                m_playlist.swapItemsAt(0, swapIndex);
            }
        }
        
        // If RepeatOff is active and the track finished naturally (not a manual skip),
        // we might want to stop playback. But to be safe and fulfill "throw to start of list",
        // we will just loop and play.
        if (m_repeatMode == RepeatMode::RepeatOff) {
            // Optional: if we want to pause at the beginning of the list instead of playing:
            // But usually users want it to continue if they manually skipped.
            // We'll just loop and play it.
        }
    }
    
    playTrack(nextIndex);
}

void AudioPlayer::previous() {
    if (m_playlist.isEmpty()) return;
    
    // If we're past 3 seconds, restart the song first
    if (position() > 3000) {
        seek(0);
        return;
    }
    
    int prevIndex = m_currentIndex - 1;
    if (prevIndex < 0) {
        // Also loop to the end if we hit previous on the first track
        prevIndex = m_playlist.size() - 1;
    }
    
    playTrack(prevIndex);
}

void AudioPlayer::seek(qint64 positionMs) {
    m_player->setPosition(positionMs);
}

void AudioPlayer::setVolume(int volumePercent) {
    if (volumePercent < 0) volumePercent = 0;
    if (volumePercent > 100) volumePercent = 100;
    
    // Convert to linear volume factor [0.0, 1.0]
    double factor = static_cast<double>(volumePercent) / 100.0;
    m_audioOutput->setVolume(factor);
    emit volumeChanged(volumePercent);
}

int AudioPlayer::volume() const {
    return qRound(m_audioOutput->volume() * 100.0);
}

void AudioPlayer::toggleMute() {
    if (m_isMuted) {
        m_isMuted = false;
        setVolume(m_previousVolume);
    } else {
        m_previousVolume = volume();
        m_isMuted = true;
        m_audioOutput->setVolume(0.0);
        emit volumeChanged(0);
    }
}

bool AudioPlayer::isMuted() const {
    return m_isMuted;
}

void AudioPlayer::setShuffle(bool enabled) {
    if (m_shuffle == enabled) return;
    m_shuffle = enabled;
    
    // Save current playing track path to keep it playing
    QString currentPath;
    if (m_currentIndex >= 0 && m_currentIndex < m_playlist.size()) {
        currentPath = m_playlist[m_currentIndex].filePath;
    }
    
    if (m_shuffle) {
        shufflePlaylist();
    } else {
        m_playlist = m_originalPlaylist;
    }
    
    // Relocate current track index
    if (!currentPath.isEmpty()) {
        for (int i = 0; i < m_playlist.size(); ++i) {
            if (m_playlist[i].filePath == currentPath) {
                if (m_shuffle && i != 0) {
                    m_playlist.swapItemsAt(0, i);
                    m_currentIndex = 0;
                } else {
                    m_currentIndex = i;
                }
                break;
            }
        }
    }
    
    emit shuffleChanged(m_shuffle);
    emit playlistChanged();
}

bool AudioPlayer::isShuffle() const {
    return m_shuffle;
}

void AudioPlayer::setRepeatMode(RepeatMode mode) {
    if (m_repeatMode == mode) return;
    m_repeatMode = mode;
    emit repeatModeChanged(m_repeatMode);
}

AudioPlayer::RepeatMode AudioPlayer::repeatMode() const {
    return m_repeatMode;
}

bool AudioPlayer::isPlaying() const {
    return m_player->playbackState() == QMediaPlayer::PlayingState;
}

qint64 AudioPlayer::position() const {
    return m_player->position();
}

qint64 AudioPlayer::duration() const {
    return m_player->duration();
}

int AudioPlayer::currentIndex() const {
    return m_currentIndex;
}

TrackMetadata AudioPlayer::currentTrack() const {
    if (m_currentIndex >= 0 && m_currentIndex < m_playlist.size()) {
        return m_playlist[m_currentIndex];
    }
    return TrackMetadata();
}

QList<TrackMetadata> AudioPlayer::playlist() const {
    return m_playlist;
}

void AudioPlayer::shufflePlaylist() {
    if (m_playlist.size() <= 1) return;
    
    // Fisher-Yates shuffle
    for (int i = m_playlist.size() - 1; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        m_playlist.swapItemsAt(i, j);
    }
}

void AudioPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::EndOfMedia) {
        if (m_repeatMode == RepeatMode::RepeatOne) {
            seek(0);
            m_player->play();
        } else {
            next();
        }
    }
}

void AudioPlayer::onPositionChanged(qint64 pos) {
    emit positionChanged(pos);
}

void AudioPlayer::onDurationChanged(qint64 dur) {
    emit durationChanged(dur);
}

void AudioPlayer::onPlaybackStateChanged(QMediaPlayer::PlaybackState state) {
    emit playbackStateChanged(state);
}
