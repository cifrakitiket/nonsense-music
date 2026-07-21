#include "MiniPlayer.h"
#include "StyleManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QFile>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QEvent>
#include <QStyle>

static QIcon createPlaybackIcon(const QString &type, const QColor &color) {
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(color);
    p.setPen(Qt::NoPen);
    
    if (type == "prev") {
        QPolygon poly; poly << QPoint(20, 8) << QPoint(10, 16) << QPoint(20, 24);
        p.drawPolygon(poly);
        p.drawRect(8, 8, 3, 16);
    } else if (type == "next") {
        QPolygon poly; poly << QPoint(12, 8) << QPoint(22, 16) << QPoint(12, 24);
        p.drawPolygon(poly);
        p.drawRect(21, 8, 3, 16);
    } else if (type == "play") {
        QPolygon poly; poly << QPoint(12, 6) << QPoint(24, 16) << QPoint(12, 26);
        p.drawPolygon(poly);
    } else if (type == "pause") {
        p.drawRect(10, 8, 4, 16);
        p.drawRect(18, 8, 4, 16);
    }
    return QIcon(pix);
}

MiniPlayer::MiniPlayer(AudioPlayer *player, QWidget *parent)
    : QWidget(parent), m_audioPlayer(player), m_mainWindow(nullptr) {
    
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    
    setupUI();
    applyQSS();
    
    connect(m_audioPlayer, &AudioPlayer::trackChanged, this, &MiniPlayer::updateTrackInfo);
    connect(m_audioPlayer, &AudioPlayer::playbackStateChanged, this, &MiniPlayer::updatePlaybackState);
    connect(m_audioPlayer, &AudioPlayer::positionChanged, this, &MiniPlayer::updatePosition);
    connect(m_audioPlayer, &AudioPlayer::durationChanged, this, &MiniPlayer::updateDuration);
    connect(m_audioPlayer, &AudioPlayer::shuffleChanged, this, &MiniPlayer::updateShuffle);
    connect(m_audioPlayer, &AudioPlayer::repeatModeChanged, this, &MiniPlayer::updateRepeat);
    
    connect(m_playPauseBtn, &QPushButton::clicked, this, &MiniPlayer::onPlayPauseClicked);
    connect(m_prevBtn, &QPushButton::clicked, m_audioPlayer, &AudioPlayer::previous);
    connect(m_nextBtn, &QPushButton::clicked, m_audioPlayer, &AudioPlayer::next);
    connect(m_shuffleBtn, &QPushButton::clicked, this, &MiniPlayer::onShuffleClicked);
    connect(m_repeatBtn, &QPushButton::clicked, this, &MiniPlayer::onRepeatClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &MiniPlayer::onCloseClicked);
    connect(m_pinBtn, &QPushButton::clicked, this, &MiniPlayer::onPinClicked);
    connect(m_progressSlider, &QSlider::sliderMoved, this, &MiniPlayer::onSliderMoved);
    connect(m_progressSlider, &QSlider::sliderPressed, this, [this]() { m_isSliderTracking = true; });
    connect(m_progressSlider, &QSlider::sliderReleased, this, [this]() { 
        m_isSliderTracking = false; 
        onSliderMoved(m_progressSlider->value());
    });
    
    updateTrackInfo(m_audioPlayer->currentTrack());
    updateDuration(m_audioPlayer->duration());
    updatePlaybackState(m_audioPlayer->isPlaying() ? QMediaPlayer::PlayingState : QMediaPlayer::StoppedState);
    updateShuffle(m_audioPlayer->isShuffle());
    updateRepeat(m_audioPlayer->repeatMode());
    
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);
    QPropertyAnimation *fadeIn = new QPropertyAnimation(effect, "opacity");
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
}

MiniPlayer::~MiniPlayer() {}

void MiniPlayer::setMainWindow(QWidget *mainWin) {
    m_mainWindow = mainWin;
}

void MiniPlayer::setupUI() {
    setFixedSize(360, 160);
    
    QWidget *container = new QWidget(this);
    container->setObjectName("MiniPlayerContainer");
    container->setGeometry(0, 0, 360, 160);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(16, 8, 16, 16);
    mainLayout->setSpacing(8);
    
    // Top Row: Pin and Close
    QWidget *topRow = new QWidget(container);
    QHBoxLayout *topLayout = new QHBoxLayout(topRow);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(8);
    
    m_pinBtn = new QPushButton(QString::fromUtf8("\xF0\x9F\x93\x8C"), topRow); // Pushpin
    m_pinBtn->setObjectName("UtilityBtn");
    m_pinBtn->setFixedSize(20, 20);
    m_pinBtn->setCursor(Qt::PointingHandCursor);
    m_pinBtn->setCheckable(true);
    m_pinBtn->setChecked(true); // Default stay on top
    
    m_closeBtn = new QPushButton(QString("\u2715"), topRow);
    m_closeBtn->setObjectName("UtilityBtn");
    m_closeBtn->setFixedSize(20, 20);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    
    topLayout->addStretch();
    topLayout->addWidget(m_pinBtn);
    topLayout->addWidget(m_closeBtn);
    
    // Middle Row: Cover + Text
    QWidget *middleRow = new QWidget(container);
    QHBoxLayout *middleLayout = new QHBoxLayout(middleRow);
    middleLayout->setContentsMargins(0, 0, 0, 0);
    middleLayout->setSpacing(12);
    
    m_coverLabel = new QLabel(middleRow);
    m_coverLabel->setFixedSize(56, 56);
    m_coverLabel->setAlignment(Qt::AlignCenter);
    m_coverLabel->setText(QString("\u266B"));
    
    QWidget *textCol = new QWidget(middleRow);
    QVBoxLayout *textLayout = new QVBoxLayout(textCol);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(2);
    textLayout->setAlignment(Qt::AlignVCenter);
    
    m_titleLabel = new QLabel("-", textCol);
    m_titleLabel->setObjectName("MiniTitle");
    m_titleLabel->setStyleSheet(QString("font-weight: bold; color: %1; font-size: 14px;").arg(StyleManager::textPrimary()));
    
    m_artistLabel = new QLabel("-", textCol);
    m_artistLabel->setObjectName("MiniArtist");
    m_artistLabel->setStyleSheet(QString("color: %1; font-size: 12px;").arg(StyleManager::textSecondary()));
    
    textLayout->addWidget(m_titleLabel);
    textLayout->addWidget(m_artistLabel);
    
    middleLayout->addWidget(m_coverLabel);
    middleLayout->addWidget(textCol);
    middleLayout->addStretch();
    
    // Progress Slider Row
    QWidget *sliderRow = new QWidget(container);
    QHBoxLayout *sliderLayout = new QHBoxLayout(sliderRow);
    sliderLayout->setContentsMargins(0, 0, 0, 0);
    sliderLayout->setSpacing(8);
    
    m_timeElapsedLabel = new QLabel("0:00", sliderRow);
    m_timeElapsedLabel->setStyleSheet("color: transparent; font-size: 10px;");
    m_timeElapsedLabel->setFixedWidth(30);
    m_timeElapsedLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    m_progressSlider = new QSlider(Qt::Horizontal, sliderRow);
    m_progressSlider->setRange(0, 1000);
    m_progressSlider->setValue(0);
    m_progressSlider->setCursor(Qt::PointingHandCursor);
    m_progressSlider->setFixedHeight(12);
    m_progressSlider->installEventFilter(this);
    
    m_timeTotalLabel = new QLabel("0:00", sliderRow);
    m_timeTotalLabel->setStyleSheet("color: transparent; font-size: 10px;");
    m_timeTotalLabel->setFixedWidth(30);
    m_timeTotalLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    
    sliderLayout->addWidget(m_timeElapsedLabel);
    sliderLayout->addWidget(m_progressSlider);
    sliderLayout->addWidget(m_timeTotalLabel);
    
    sliderRow->installEventFilter(this);
    
    // Bottom Row: Controls
    QWidget *controlsRow = new QWidget(container);
    QHBoxLayout *controlsLayout = new QHBoxLayout(controlsRow);
    controlsLayout->setContentsMargins(0, 0, 0, 0);
    controlsLayout->setSpacing(16);
    controlsLayout->setAlignment(Qt::AlignCenter);
    
    m_shuffleBtn = new QPushButton(controlsRow);
    m_shuffleBtn->setFixedSize(32, 32);
    m_shuffleBtn->setCheckable(true);
    m_shuffleBtn->setCursor(Qt::PointingHandCursor);

    m_prevBtn = new QPushButton(controlsRow);
    m_prevBtn->setIcon(createPlaybackIcon("prev", QColor(StyleManager::textPrimary())));
    m_prevBtn->setIconSize(QSize(24, 24));
    m_prevBtn->setFixedSize(32, 32);
    m_prevBtn->setCursor(Qt::PointingHandCursor);

    m_playPauseBtn = new QPushButton(controlsRow);
    m_playPauseBtn->setObjectName("PlayPauseButton");
    m_playPauseBtn->setIcon(createPlaybackIcon("play", QColor(StyleManager::bgDeepest())));
    m_playPauseBtn->setIconSize(QSize(24, 24));
    m_playPauseBtn->setFixedSize(44, 44);
    m_playPauseBtn->setCursor(Qt::PointingHandCursor);

    m_nextBtn = new QPushButton(controlsRow);
    m_nextBtn->setIcon(createPlaybackIcon("next", QColor(StyleManager::textPrimary())));
    m_nextBtn->setIconSize(QSize(24, 24));
    m_nextBtn->setFixedSize(32, 32);
    m_nextBtn->setCursor(Qt::PointingHandCursor);

    m_repeatBtn = new QPushButton(QString("\u21BB"), controlsRow);
    m_repeatBtn->setObjectName("ControlBtn");
    m_repeatBtn->setFixedSize(24, 24);
    m_repeatBtn->setCursor(Qt::PointingHandCursor);
    
    m_likeBtn = new QPushButton(QString("\u2665"), controlsRow);
    m_likeBtn->setObjectName("ControlBtn");
    m_likeBtn->setFixedSize(24, 24);
    m_likeBtn->setCursor(Qt::PointingHandCursor);
    m_likeBtn->setCheckable(true);
    
    controlsLayout->addWidget(m_shuffleBtn);
    controlsLayout->addWidget(m_prevBtn);
    controlsLayout->addWidget(m_playPauseBtn);
    controlsLayout->addWidget(m_nextBtn);
    controlsLayout->addWidget(m_repeatBtn);
    controlsLayout->addWidget(m_likeBtn);
    
    mainLayout->addWidget(topRow);
    mainLayout->addWidget(middleRow);
    mainLayout->addWidget(sliderRow);
    mainLayout->addWidget(controlsRow);
}

QString MiniPlayer::formatTime(qint64 ms) const {
    qint64 totalSeconds = ms / 1000;
    qint64 minutes = totalSeconds / 60;
    qint64 seconds = totalSeconds % 60;
    return QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
}

void MiniPlayer::updateTrackInfo(const TrackMetadata &track) {
    if (track.filePath.isEmpty()) {
        m_titleLabel->setText("No Song");
        m_artistLabel->setText("-");
        m_coverLabel->setText(QString("\u266B"));
        return;
    }
    
    QFontMetrics titleMetrics(m_titleLabel->font());
    m_titleLabel->setText(titleMetrics.elidedText(track.title, Qt::ElideRight, 220));
    m_titleLabel->setToolTip(track.title);
    
    QFontMetrics artistMetrics(m_artistLabel->font());
    m_artistLabel->setText(artistMetrics.elidedText(track.artist, Qt::ElideRight, 220));
    m_artistLabel->setToolTip(track.artist);
    
    QString coverPath = track.coverMimeType;
    if (coverPath.isEmpty() || !QFile::exists(coverPath)) {
        QByteArray hash = QCryptographicHash::hash(track.filePath.toUtf8(), QCryptographicHash::Md5);
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/covers/";
        coverPath = cacheDir + hash.toHex() + ".jpg";
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
            path.addRoundedRect(0, 0, 56, 56, 6, 6);
            painter.setClipPath(path);
            int x = (56 - scaled.width()) / 2;
            int y = (56 - scaled.height()) / 2;
            painter.drawPixmap(x, y, scaled);
            m_coverLabel->setPixmap(rounded);
        } else {
            m_coverLabel->setText(QString("\u266B"));
        }
    } else {
        m_coverLabel->setText(QString("\u266B"));
    }
}

void MiniPlayer::updatePlaybackState(QMediaPlayer::PlaybackState state) {
    if (state == QMediaPlayer::PlayingState) {
        m_playPauseBtn->setIcon(createPlaybackIcon("pause", QColor(StyleManager::bgDeepest())));
    } else {
        m_playPauseBtn->setIcon(createPlaybackIcon("play", QColor(StyleManager::bgDeepest())));
    }
}

void MiniPlayer::updatePosition(qint64 pos) {
    if (m_isSliderTracking) return;
    
    m_timeElapsedLabel->setText(formatTime(pos));
    if (m_currentDuration > 0) {
        double ratio = static_cast<double>(pos) / m_currentDuration;
        m_progressSlider->setValue(qRound(ratio * 1000.0));
    } else {
        m_progressSlider->setValue(0);
    }
}

void MiniPlayer::updateDuration(qint64 dur) {
    m_currentDuration = dur;
    m_timeTotalLabel->setText(formatTime(dur));
}

void MiniPlayer::updateShuffle(bool enabled) {
    m_shuffleBtn->blockSignals(true);
    m_shuffleBtn->setChecked(enabled);
    m_shuffleBtn->blockSignals(false);
    
    if (enabled) {
        m_shuffleBtn->setStyleSheet(QString("color: %1;").arg(StyleManager::accent()));
    } else {
        m_shuffleBtn->setStyleSheet(""); // reset to default #ControlBtn
    }
}

void MiniPlayer::updateRepeat(AudioPlayer::RepeatMode mode) {
    if (mode == AudioPlayer::RepeatMode::RepeatOff) {
        m_repeatBtn->setText(QString("\u21BB"));
        m_repeatBtn->setStyleSheet("");
    } else if (mode == AudioPlayer::RepeatMode::RepeatOne) {
        m_repeatBtn->setText(QString("\u21BA"));
        m_repeatBtn->setStyleSheet(QString("color: %1;").arg(StyleManager::accent()));
    } else if (mode == AudioPlayer::RepeatMode::RepeatAll) {
        m_repeatBtn->setText(QString("\u21BB"));
        m_repeatBtn->setStyleSheet(QString("color: %1;").arg(StyleManager::accent()));
    }
}

void MiniPlayer::onSliderMoved(int position) {
    if (m_currentDuration > 0 && m_audioPlayer) {
        double ratio = position / 1000.0;
        m_audioPlayer->seek(static_cast<qint64>(ratio * m_currentDuration));
    }
}

void MiniPlayer::onPlayPauseClicked() {
    if (m_audioPlayer->isPlaying()) {
        m_audioPlayer->pause();
    } else {
        m_audioPlayer->play();
    }
}

void MiniPlayer::onShuffleClicked() {
    if (m_audioPlayer) {
        m_audioPlayer->setShuffle(!m_audioPlayer->isShuffle());
    }
}

void MiniPlayer::onRepeatClicked() {
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

void MiniPlayer::onCloseClicked() {
    hide();
}

void MiniPlayer::onPinClicked() {
    if (m_pinBtn->isChecked()) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    } else {
        setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
    }
    show(); // Needs to be shown again after changing flags
}

void MiniPlayer::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void MiniPlayer::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

bool MiniPlayer::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_progressSlider && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            int val = QStyle::sliderValueFromPosition(m_progressSlider->minimum(), m_progressSlider->maximum(), mouseEvent->pos().x(), m_progressSlider->width());
            m_progressSlider->setValue(val);
            onSliderMoved(val);
            return true;
        }
    } else if (obj == m_progressSlider->parent() && event->type() == QEvent::Enter) {
        m_timeElapsedLabel->setStyleSheet(QString("color: %1; font-size: 10px;").arg(StyleManager::textSecondary()));
        m_timeTotalLabel->setStyleSheet(QString("color: %1; font-size: 10px;").arg(StyleManager::textSecondary()));
    } else if (obj == m_progressSlider->parent() && event->type() == QEvent::Leave) {
        m_timeElapsedLabel->setStyleSheet("color: transparent; font-size: 10px;");
        m_timeTotalLabel->setStyleSheet("color: transparent; font-size: 10px;");
    }
    return QWidget::eventFilter(obj, event);
}

void MiniPlayer::applyQSS() {
    setStyleSheet(
        QString(
        "QWidget#MiniPlayerContainer {"
        "   background-color: %1;"
        "   border: 1px solid %2;"
        "   border-radius: 12px;"
        "}"
        "QPushButton#UtilityBtn {"
        "   background-color: transparent;"
        "   color: %3;"
        "   border: none;"
        "   font-size: 14px;"
        "}"
        "QPushButton#UtilityBtn:hover {"
        "   color: %4;"
        "}"
        "QPushButton#ControlBtn {"
        "   background-color: transparent;"
        "   color: %3;"
        "   border: none;"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "}"
        "QPushButton#ControlBtn:hover {"
        "   color: %4;"
        "}"
        "QPushButton#PlayPauseButton {"
        "   background-color: %4;"
        "   border: none;"
        "   border-radius: 22px;"
        "}"
        ).arg(StyleManager::bgElevated(), StyleManager::border(),
              StyleManager::textSecondary(), StyleManager::textPrimary())
    );

    if (m_audioPlayer && m_audioPlayer->isPlaying()) {
        m_playPauseBtn->setIcon(createPlaybackIcon("pause", QColor(StyleManager::bgDeepest())));
    } else {
        m_playPauseBtn->setIcon(createPlaybackIcon("play", QColor(StyleManager::bgDeepest())));
    }
    m_prevBtn->setIcon(createPlaybackIcon("prev", QColor(StyleManager::textPrimary())));
    m_nextBtn->setIcon(createPlaybackIcon("next", QColor(StyleManager::textPrimary())));
    
    m_progressSlider->setStyleSheet(StyleManager::sliderStyle());
    m_coverLabel->setStyleSheet(QString("background-color: %1; border-radius: 6px; font-size: 24px; color: %2;").arg(StyleManager::bgSurface(), StyleManager::textMuted()));
    m_titleLabel->setStyleSheet(QString("font-weight: bold; color: %1; font-size: 14px;").arg(StyleManager::textPrimary()));
    m_artistLabel->setStyleSheet(QString("color: %1; font-size: 12px;").arg(StyleManager::textSecondary()));
}

void MiniPlayer::refreshStyle() {
    applyQSS();
}
