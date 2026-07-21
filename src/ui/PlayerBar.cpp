#include "PlayerBar.h"
#include "StyleManager.h"
#include <QPainter>
#include <QPainterPath>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QDebug>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QEvent>
#include <QMouseEvent>
#include <QStyle>
#include "IconProvider.h"

PlayerBar::PlayerBar(QWidget *parent) : QWidget(parent) {
    setupUI();
    applyQSS();
}

PlayerBar::~PlayerBar() {}


void PlayerBar::setupUI() {
    setMinimumHeight(80);
    setMaximumHeight(96);
    setObjectName("PlayerBarContainer");

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(24, 0, 24, 0);
    mainLayout->setSpacing(0);
    
    // ── LEFT PANEL: cover + title + artist ──────────────────────────────────
    QWidget *leftPanel = new QWidget(this);
    QHBoxLayout *leftLayout = new QHBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(12);

    m_coverLabel = new QLabel(this);
    m_coverLabel->setFixedSize(56, 56);
    m_coverLabel->setAlignment(Qt::AlignCenter);
    m_coverLabel->setStyleSheet(
        QString("background-color: %1; border-radius: 4px; font-size: 20px; color: %2;")
        .arg(StyleManager::bgSurface(), StyleManager::textMuted()));

    QWidget *textContainer = new QWidget(this);
    textContainer->setStyleSheet("background: transparent;");
    QVBoxLayout *textLayout = new QVBoxLayout(textContainer);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(3);
    textLayout->setAlignment(Qt::AlignVCenter);

    m_titleLabel = new QLabel("No Song Playing", this);
    m_titleLabel->setObjectName("PlayerBarTitle");
    m_titleLabel->setStyleSheet(
        QString("color: %1; font-size: 14px; font-weight: bold; background: transparent;")
        .arg(StyleManager::textPrimary()));

    m_artistLabel = new QLabel("-", this);
    m_artistLabel->setObjectName("PlayerBarArtist");
    m_artistLabel->setStyleSheet(
        QString("color: %1; font-size: 12px; background: transparent;")
        .arg(StyleManager::textSecondary()));

    textLayout->addWidget(m_titleLabel);
    textLayout->addWidget(m_artistLabel);

    leftLayout->addWidget(m_coverLabel);
    leftLayout->addWidget(textContainer);
    leftLayout->addStretch();
    
    // ── CENTER PANEL: controls + progress ──────────────────────────────────
    QWidget *centerPanel = new QWidget(this);
    QVBoxLayout *centerLayout = new QVBoxLayout(centerPanel);
    centerLayout->setContentsMargins(0, 8, 0, 8);
    centerLayout->setSpacing(4);

    QWidget *btnRow = new QWidget(centerPanel);
    QHBoxLayout *btnLayout = new QHBoxLayout(btnRow);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(20);
    btnLayout->setAlignment(Qt::AlignCenter);

    m_shuffleBtn = new QPushButton(btnRow);
    m_shuffleBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::Shuffle, QSize(24,24), QColor(StyleManager::textSecondary()))));
    m_shuffleBtn->setIconSize(QSize(20,20));
    m_shuffleBtn->setFixedSize(32, 32);
    m_shuffleBtn->setCheckable(true);
    m_shuffleBtn->setCursor(Qt::PointingHandCursor);

    m_prevBtn = new QPushButton(btnRow);
    m_prevBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::Previous, QSize(24,24), QColor(StyleManager::textPrimary()))));
    m_prevBtn->setIconSize(QSize(24, 24));
    m_prevBtn->setFixedSize(32, 32);
    m_prevBtn->setCursor(Qt::PointingHandCursor);

    m_playPauseBtn = new QPushButton(btnRow);
    m_playPauseBtn->setObjectName("PlayPauseButton");
    m_playPauseBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::Play, QSize(24,24), QColor(StyleManager::bgDeepest()))));
    m_playPauseBtn->setIconSize(QSize(24, 24));
    m_playPauseBtn->setFixedSize(44, 44);
    m_playPauseBtn->setCursor(Qt::PointingHandCursor);

    m_nextBtn = new QPushButton(btnRow);
    m_nextBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::Next, QSize(24,24), QColor(StyleManager::textPrimary()))));
    m_nextBtn->setIconSize(QSize(24, 24));
    m_nextBtn->setFixedSize(32, 32);
    m_nextBtn->setCursor(Qt::PointingHandCursor);

    m_repeatBtn = new QPushButton(btnRow);
    m_repeatBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::Repeat, QSize(24,24), QColor(StyleManager::textSecondary()))));
    m_repeatBtn->setIconSize(QSize(20,20));
    m_repeatBtn->setFixedSize(32, 32);
    m_repeatBtn->setCursor(Qt::PointingHandCursor);

    m_likeBtn = new QPushButton(btnRow);
    m_likeBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::HeartOutline, QSize(24,24), QColor(StyleManager::textSecondary()))));
    m_likeBtn->setIconSize(QSize(20,20));
    m_likeBtn->setFixedSize(32, 32);
    m_likeBtn->setCheckable(true);
    m_likeBtn->setCursor(Qt::PointingHandCursor);

    btnLayout->addWidget(m_shuffleBtn);
    btnLayout->addWidget(m_prevBtn);
    btnLayout->addWidget(m_playPauseBtn);
    btnLayout->addWidget(m_nextBtn);
    btnLayout->addWidget(m_repeatBtn);
    btnLayout->addWidget(m_likeBtn);
    
    QWidget *sliderRow = new QWidget(centerPanel);
    QHBoxLayout *sliderLayout = new QHBoxLayout(sliderRow);
    sliderLayout->setContentsMargins(0, 0, 0, 0);
    sliderLayout->setSpacing(10);
    
    m_timeElapsedLabel = new QLabel("0:00", sliderRow);
    m_timeElapsedLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(StyleManager::textSecondary()));
    m_timeElapsedLabel->setFixedWidth(38);
    m_timeElapsedLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    m_progressSlider = new QSlider(Qt::Horizontal, sliderRow);
    m_progressSlider->setRange(0, 1000);
    m_progressSlider->setValue(0);
    m_progressSlider->setCursor(Qt::PointingHandCursor);
    m_progressSlider->installEventFilter(this);
    
    m_timeTotalLabel = new QLabel("0:00", sliderRow);
    m_timeTotalLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(StyleManager::textSecondary()));
    m_timeTotalLabel->setFixedWidth(38);
    m_timeTotalLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    
    sliderLayout->addWidget(m_timeElapsedLabel);
    sliderLayout->addWidget(m_progressSlider, 1);
    sliderLayout->addWidget(m_timeTotalLabel);
    
    centerLayout->addWidget(btnRow);
    centerLayout->addWidget(sliderRow);
    
    // --- RIGHT PANEL: Volume / Utility ---
    QWidget *rightPanel = new QWidget(this);
    QHBoxLayout *rightLayout = new QHBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(10);
    rightLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    m_volumeBtn = new QPushButton(rightPanel);
    m_volumeBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::VolumeOn, QSize(24,24), QColor(StyleManager::textSecondary()))));
    m_volumeBtn->setIconSize(QSize(20,20));
    m_volumeBtn->setMinimumSize(24, 24);
    m_volumeBtn->setMaximumSize(36, 36);
    m_volumeBtn->setCursor(Qt::PointingHandCursor);
    m_volumeBtn->setStyleSheet(QString("background: transparent; border: none;"));
    
    m_volumeSlider = new QSlider(Qt::Horizontal, rightPanel);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(100);
    m_volumeSlider->setMinimumWidth(80);
    m_volumeSlider->setMaximumWidth(150);
    m_volumeSlider->setCursor(Qt::PointingHandCursor);
    
    m_miniPlayerBtn = new QPushButton(rightPanel);
    m_miniPlayerBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::MiniPlayer, QSize(24,24), QColor(StyleManager::textSecondary()))));
    m_miniPlayerBtn->setIconSize(QSize(20,20));
    m_miniPlayerBtn->setMinimumSize(24, 24);
    m_miniPlayerBtn->setMaximumSize(36, 36);
    m_miniPlayerBtn->setCursor(Qt::PointingHandCursor);
    m_miniPlayerBtn->setStyleSheet(QString("background: transparent; border: none;"));
    m_miniPlayerBtn->setToolTip("Mini Player");
    
    rightLayout->addWidget(m_volumeBtn);
    rightLayout->addWidget(m_volumeSlider);
    rightLayout->addWidget(m_miniPlayerBtn);
    
    leftPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    centerPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    rightPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // Stretch ratios: left=3, center=5, right=3 (matching Spotify proportions)
    mainLayout->addWidget(leftPanel, 3);
    mainLayout->addWidget(centerPanel, 5);
    mainLayout->addWidget(rightPanel, 3);
    
    connect(m_playPauseBtn, &QPushButton::clicked, this, &PlayerBar::onPlayPauseClicked);
    connect(m_progressSlider, &QSlider::sliderMoved, this, &PlayerBar::onSliderMoved);
    connect(m_progressSlider, &QSlider::sliderPressed, this, [this]() { m_isSliderTracking = true; });
    connect(m_progressSlider, &QSlider::sliderReleased, this, [this]() { 
        m_isSliderTracking = false; 
        onSliderMoved(m_progressSlider->value());
    });
    connect(m_volumeSlider, &QSlider::sliderMoved, this, &PlayerBar::onVolumeSliderMoved);
    connect(m_volumeBtn, &QPushButton::clicked, this, [this]() {
        if (m_audioPlayer) m_audioPlayer->toggleMute();
    });
    connect(m_shuffleBtn, &QPushButton::clicked, this, &PlayerBar::onShuffleClicked);
    connect(m_repeatBtn, &QPushButton::clicked, this, &PlayerBar::onRepeatClicked);
    connect(m_miniPlayerBtn, &QPushButton::clicked, this, &PlayerBar::onMiniPlayerClicked);
}

bool PlayerBar::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_progressSlider && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            int val = QStyle::sliderValueFromPosition(m_progressSlider->minimum(), m_progressSlider->maximum(), mouseEvent->pos().x(), m_progressSlider->width());
            m_progressSlider->setValue(val);
            onSliderMoved(val);
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void PlayerBar::setAudioPlayer(AudioPlayer *player) {
    // Disconnect from the previous player to avoid duplicate signal connections.
    // Without this, calling setAudioPlayer() a second time (e.g. from onEditRequested)
    // would cause every signal to fire twice, tripling UI updates on each call.
    if (m_audioPlayer) {
        disconnect(m_audioPlayer, nullptr, this, nullptr);
        disconnect(m_prevBtn, nullptr, m_audioPlayer, nullptr);
        disconnect(m_nextBtn, nullptr, m_audioPlayer, nullptr);
    }
    
    m_audioPlayer = player;
    if (!m_audioPlayer) return;
    
    connect(m_audioPlayer, &AudioPlayer::trackChanged, this, &PlayerBar::updateTrackInfo);
    connect(m_audioPlayer, &AudioPlayer::playbackStateChanged, this, &PlayerBar::updatePlaybackState);
    connect(m_audioPlayer, &AudioPlayer::positionChanged, this, &PlayerBar::updatePosition);
    connect(m_audioPlayer, &AudioPlayer::durationChanged, this, &PlayerBar::updateDuration);
    connect(m_audioPlayer, &AudioPlayer::volumeChanged, this, &PlayerBar::updateVolume);
    connect(m_audioPlayer, &AudioPlayer::shuffleChanged, this, &PlayerBar::updateShuffle);
    connect(m_audioPlayer, &AudioPlayer::repeatModeChanged, this, &PlayerBar::updateRepeat);
    
    connect(m_prevBtn, &QPushButton::clicked, m_audioPlayer, &AudioPlayer::previous);
    connect(m_nextBtn, &QPushButton::clicked, m_audioPlayer, &AudioPlayer::next);
    
    updateVolume(m_audioPlayer->volume());
    updateShuffle(m_audioPlayer->isShuffle());
    updateRepeat(m_audioPlayer->repeatMode());
}

void PlayerBar::updateTrackInfo(const TrackMetadata &track) {
    m_titleLabel->setText(track.title);
    m_artistLabel->setText(track.artist);
    
    QString coverPath = track.coverMimeType;
    if (coverPath.isEmpty() || !QFile::exists(coverPath)) {
        if (m_audioPlayer) {
            QByteArray hash = QCryptographicHash::hash(track.filePath.toUtf8(), QCryptographicHash::Md5);
            QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/covers/";
            coverPath = cacheDir + hash.toHex() + ".jpg";
        }
    }
    
    if (QFile::exists(coverPath)) {
        QPixmap pm(coverPath);
        if (!pm.isNull()) {
            QPixmap scaled = pm.scaled(56, 56, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QPixmap rounded(56, 56);
            rounded.fill(Qt::transparent);
            QPainter painter(&rounded);
            painter.setRenderHint(QPainter::Antialiasing);
            QPainterPath path;
            path.addRoundedRect(0, 0, 56, 56, 8, 8);
            painter.setClipPath(path);
            int x = (56 - scaled.width()) / 2;
            int y = (56 - scaled.height()) / 2;
            painter.drawPixmap(x, y, scaled);
            m_coverLabel->setPixmap(rounded);
            m_coverLabel->setText("");
            m_coverLabel->setPixmap(IconProvider::getPixmap(IconType::Note, QSize(32,32), QColor(StyleManager::textMuted())));
        }
    } else {
        m_coverLabel->setText("");
        m_coverLabel->setPixmap(IconProvider::getPixmap(IconType::Note, QSize(32,32), QColor(StyleManager::textMuted())));
    }
}

void PlayerBar::updatePlaybackState(QMediaPlayer::PlaybackState state) {
    if (state == QMediaPlayer::PlayingState) {
        m_playPauseBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::Pause, QSize(24,24), QColor(StyleManager::bgDeepest()))));
    } else {
        m_playPauseBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::Play, QSize(24,24), QColor(StyleManager::bgDeepest()))));
    }
}

void PlayerBar::updatePosition(qint64 pos) {
    if (m_isSliderTracking) return;
    
    m_timeElapsedLabel->setText(formatTime(pos));
    if (m_currentDuration > 0) {
        double ratio = static_cast<double>(pos) / m_currentDuration;
        m_progressSlider->setValue(qRound(ratio * 1000.0));
    } else {
        m_progressSlider->setValue(0);
    }
}

void PlayerBar::updateDuration(qint64 dur) {
    m_currentDuration = dur;
    m_timeTotalLabel->setText(formatTime(dur));
}

void PlayerBar::updateVolume(int volume) {
    m_volumeSlider->blockSignals(true);
    m_volumeSlider->setValue(volume);
    m_volumeSlider->blockSignals(false);
    
    if (volume == 0) m_volumeBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::VolumeMute, QSize(24,24), QColor(StyleManager::textSecondary()))));
    else m_volumeBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::VolumeOn, QSize(24,24), QColor(StyleManager::textSecondary()))));
}

void PlayerBar::updateShuffle(bool enabled) {
    m_shuffleBtn->blockSignals(true);
    m_shuffleBtn->setChecked(enabled);
    m_shuffleBtn->blockSignals(false);
    
    if (enabled) {
        m_shuffleBtn->setStyleSheet(QString("color: %1; font-size: 16px; background: transparent; border: none; font-weight: bold;").arg(StyleManager::accent()));
    } else {
        m_shuffleBtn->setStyleSheet(QString("color: %1; font-size: 16px; background: transparent; border: none; font-weight: bold;").arg(StyleManager::textSecondary()));
    }
}

void PlayerBar::updateRepeat(AudioPlayer::RepeatMode mode) {
    if (mode == AudioPlayer::RepeatMode::RepeatOff) {
        m_repeatBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::Repeat, QSize(24,24), QColor(StyleManager::textSecondary()))));
    } else if (mode == AudioPlayer::RepeatMode::RepeatOne) {
        m_repeatBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::RepeatOne, QSize(24,24), QColor(StyleManager::accent()))));
    } else if (mode == AudioPlayer::RepeatMode::RepeatAll) {
        m_repeatBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::Repeat, QSize(24,24), QColor(StyleManager::accent()))));
    }
}

void PlayerBar::onPlayPauseClicked() {
    if (!m_audioPlayer) return;
    
    // Scale animation on play/pause
    QPropertyAnimation *anim = new QPropertyAnimation(m_playPauseBtn, "geometry");
    anim->setDuration(200);
    QRect startGeom = m_playPauseBtn->geometry();
    QRect targetCenter = startGeom.adjusted(-2, -2, 2, 2);
    anim->setKeyValueAt(0.0, startGeom);
    anim->setKeyValueAt(0.5, targetCenter);
    anim->setKeyValueAt(1.0, startGeom);
    anim->setEasingCurve(QEasingCurve::OutBack);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    
    if (m_audioPlayer->isPlaying()) {
        m_audioPlayer->pause();
    } else {
        m_audioPlayer->play();
    }
}

void PlayerBar::onSliderMoved(int position) {
    if (!m_audioPlayer || m_currentDuration <= 0) return;
    double ratio = static_cast<double>(position) / 1000.0;
    qint64 targetMs = qRound(ratio * m_currentDuration);
    m_audioPlayer->seek(targetMs);
    m_timeElapsedLabel->setText(formatTime(targetMs));
}

void PlayerBar::onVolumeSliderMoved(int value) {
    if (m_audioPlayer) {
        m_audioPlayer->setVolume(value);
    }
}

void PlayerBar::onShuffleClicked() {
    if (m_audioPlayer) {
        m_audioPlayer->setShuffle(!m_audioPlayer->isShuffle());
    }
}

void PlayerBar::onRepeatClicked() {
    if (!m_audioPlayer) return;
    auto mode = m_audioPlayer->repeatMode();
    if (mode == AudioPlayer::RepeatMode::RepeatOff) {
        m_audioPlayer->setRepeatMode(AudioPlayer::RepeatMode::RepeatAll);
    } else if (mode == AudioPlayer::RepeatMode::RepeatAll) {
        m_audioPlayer->setRepeatMode(AudioPlayer::RepeatMode::RepeatOne);
    } else {
        m_audioPlayer->setRepeatMode(AudioPlayer::RepeatMode::RepeatOff);
    }
}

QString PlayerBar::formatTime(qint64 ms) const {
    int totalSec = qRound(static_cast<double>(ms) / 1000.0);
    int sec = totalSec % 60;
    int min = totalSec / 60;
    return QString("%1:%2").arg(min).arg(sec, 2, 10, QChar('0'));
}

void PlayerBar::applyQSS() {
    if (m_audioPlayer && m_audioPlayer->isPlaying()) {
        m_playPauseBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::Pause, QSize(24,24), QColor(StyleManager::bgDeepest()))));
    } else {
        m_playPauseBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::Play, QSize(24,24), QColor(StyleManager::bgDeepest()))));
    }
    m_prevBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::Previous, QSize(24,24), QColor(StyleManager::textPrimary()))));
    m_nextBtn->setIcon(QIcon(IconProvider::getPixmap(IconType::Next, QSize(24,24), QColor(StyleManager::textPrimary()))));

    m_progressSlider->setStyleSheet(StyleManager::sliderStyle());
    m_volumeSlider->setStyleSheet(StyleManager::sliderStyle());
}


void PlayerBar::onMiniPlayerClicked() {
    emit miniPlayerRequested();
}

void PlayerBar::refreshStyle() {
    QString btnStyle = QString(
        "QPushButton { background: transparent; border: none; color: %1; font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { color: %2; }"
    ).arg(StyleManager::textSecondary(), StyleManager::textPrimary());

    m_shuffleBtn->setStyleSheet(btnStyle);
    m_prevBtn->setStyleSheet(btnStyle);
    m_nextBtn->setStyleSheet(btnStyle);
    m_repeatBtn->setStyleSheet(btnStyle);
    
    m_likeBtn->setStyleSheet(QString(
        "QPushButton { background: transparent; border: none; color: %1; font-size: 18px; }"
        "QPushButton:hover { color: %2; }"
    ).arg(StyleManager::accent(), StyleManager::accentHover()));

    m_playPauseBtn->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: %2; border-radius: 22px; font-size: 16px; }"
        "QPushButton:hover { background-color: %3; transform: scale(1.05); }"
    ).arg(StyleManager::textPrimary(), StyleManager::accent(), StyleManager::textPrimary()));

    setStyleSheet(QString("QWidget#PlayerBarContainer { background-color: %1; border-top: 1px solid %2; }")
                  .arg(StyleManager::getDynamicBg(StyleManager::bgElevated(), 180), StyleManager::border()));
    m_coverLabel->setStyleSheet(
        QString("background-color: %1; border-radius: 4px; font-size: 20px; color: %2;")
        .arg(StyleManager::bgSurface(), StyleManager::textMuted()));
    m_titleLabel->setStyleSheet(
        QString("color: %1; font-size: 14px; font-weight: bold; background: transparent;")
        .arg(StyleManager::textPrimary()));
    m_artistLabel->setStyleSheet(
        QString("color: %1; font-size: 12px; background: transparent;")
        .arg(StyleManager::textSecondary()));
    m_timeElapsedLabel->setStyleSheet(
        QString("color: %1; font-size: 11px;").arg(StyleManager::textSecondary()));
    m_timeTotalLabel->setStyleSheet(
        QString("color: %1; font-size: 11px;").arg(StyleManager::textSecondary()));
    m_volumeBtn->setStyleSheet(
        QString("background: transparent; border: none; color: %1; font-size: 16px;")
        .arg(StyleManager::textSecondary()));
    m_miniPlayerBtn->setStyleSheet(
        QString("background: transparent; border: none; color: %1; font-size: 14px;")
        .arg(StyleManager::textSecondary()));
    updateShuffle(m_audioPlayer ? m_audioPlayer->isShuffle() : false);
    updateRepeat(m_audioPlayer ? m_audioPlayer->repeatMode() : AudioPlayer::RepeatMode::RepeatOff);
}
