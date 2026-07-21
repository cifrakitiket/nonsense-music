#include "EditTrackDialog.h"
#include "StyleManager.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QDebug>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

EditTrackDialog::EditTrackDialog(const TrackMetadata &track, QWidget *parent) 
    : QDialog(parent), m_track(track) {
    setupUI();
    applyQSS();
    
    m_titleInput->setText(track.title);
    m_artistInput->setText(track.artist);
    
    QString coverPath = track.coverMimeType;
    if (coverPath.isEmpty() || !QFile::exists(coverPath)) {
        QByteArray hash = QCryptographicHash::hash(track.filePath.toUtf8(), QCryptographicHash::Md5);
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/covers/";
        coverPath = cacheDir + hash.toHex() + ".jpg";
    }
    loadCover(coverPath);
    
    // Fade-in animation
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);
    QPropertyAnimation *fadeIn = new QPropertyAnimation(effect, "opacity");
    fadeIn->setDuration(250);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
}

EditTrackDialog::~EditTrackDialog() {}

void EditTrackDialog::setupUI() {
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(460, 250);
    
    QWidget *container = new QWidget(this);
    container->setObjectName("DialogContainer");
    container->setGeometry(0, 0, 460, 250);
    
    QHBoxLayout *mainLayout = new QHBoxLayout(container);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);
    
    QVBoxLayout *coverLayout = new QVBoxLayout();
    coverLayout->setSpacing(10);
    
    m_coverLabel = new QLabel(container);
    m_coverLabel->setFixedSize(120, 120);
    m_coverLabel->setAlignment(Qt::AlignCenter);
    m_coverLabel->setStyleSheet(QString("background-color: %1; border-radius: 8px; font-size: 32px; color: %2;").arg(StyleManager::bgSurface(), StyleManager::textMuted()));
    m_coverLabel->setText(QString("\u266B"));
    
    QPushButton *changeCoverBtn = new QPushButton("Change Cover", container);
    changeCoverBtn->setMinimumHeight(26);
    changeCoverBtn->setMaximumHeight(34);
    changeCoverBtn->setObjectName("ChangeCoverButton");
    changeCoverBtn->setCursor(Qt::PointingHandCursor);
    
    coverLayout->addWidget(m_coverLabel);
    coverLayout->addWidget(changeCoverBtn);
    coverLayout->addStretch();
    
    QVBoxLayout *inputsLayout = new QVBoxLayout();
    inputsLayout->setSpacing(12);
    
    QLabel *header = new QLabel("Edit Track Details", container);
    header->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;").arg(StyleManager::textPrimary()));
    inputsLayout->addWidget(header);
    
    m_titleInput = new QLineEdit(container);
    m_titleInput->setPlaceholderText("Song Title");
    m_titleInput->setMinimumHeight(32);
    m_titleInput->setMaximumHeight(40);
    inputsLayout->addWidget(m_titleInput);

    m_artistInput = new QLineEdit(container);
    m_artistInput->setPlaceholderText("Artist Name");
    m_artistInput->setMinimumHeight(32);
    m_artistInput->setMaximumHeight(40);
    inputsLayout->addWidget(m_artistInput);
    
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    btnLayout->addStretch();
    
    m_cancelBtn = new QPushButton("Cancel", container);
    m_cancelBtn->setObjectName("CancelButton");
    m_cancelBtn->setMinimumHeight(28);
    m_cancelBtn->setMaximumHeight(36);
    m_cancelBtn->setMinimumWidth(70);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);

    m_saveBtn = new QPushButton("Save", container);
    m_saveBtn->setObjectName("SaveButton");
    m_saveBtn->setMinimumHeight(28);
    m_saveBtn->setMaximumHeight(36);
    m_saveBtn->setMinimumWidth(70);
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    
    btnLayout->addWidget(m_cancelBtn);
    btnLayout->addWidget(m_saveBtn);
    inputsLayout->addLayout(btnLayout);
    
    mainLayout->addLayout(coverLayout);
    mainLayout->addLayout(inputsLayout);
    
    connect(changeCoverBtn, &QPushButton::clicked, this, &EditTrackDialog::onChangeCoverClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_saveBtn, &QPushButton::clicked, this, &EditTrackDialog::onSaveClicked);
}

void EditTrackDialog::loadCover(const QString &path) {
    if (QFile::exists(path)) {
        QPixmap pm(path);
        if (!pm.isNull()) {
            QPixmap scaled = pm.scaled(120, 120, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            QPixmap rounded(120, 120);
            rounded.fill(Qt::transparent);
            QPainter painter(&rounded);
            painter.setRenderHint(QPainter::Antialiasing);
            QPainterPath painterPath;
            painterPath.addRoundedRect(0, 0, 120, 120, 8, 8);
            painter.setClipPath(painterPath);
            painter.drawPixmap(0, 0, scaled);
            m_coverLabel->setPixmap(rounded);
        }
    }
}

void EditTrackDialog::onChangeCoverClicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Select Cover Image", 
                                                    QStandardPaths::writableLocation(QStandardPaths::PicturesLocation), 
                                                    "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            m_newCoverData = file.readAll();
            file.close();
            loadCover(fileName);
        }
    }
}

void EditTrackDialog::onSaveClicked() {
    if (m_titleInput->text().trimmed().isEmpty()) {
        return;
    }
    accept();
}

QString EditTrackDialog::title() const {
    return m_titleInput->text().trimmed();
}

QString EditTrackDialog::artist() const {
    return m_artistInput->text().trimmed();
}

QByteArray EditTrackDialog::newCoverData() const {
    return m_newCoverData;
}

void EditTrackDialog::applyQSS() {
    setStyleSheet(
        QString(
        "QWidget#DialogContainer {"
        "   background-color: %1;"
        "   border: 1px solid %2;"
        "   border-radius: 12px;"
        "}"
        "QLineEdit {"
        "   background-color: %3;"
        "   color: %4;"
        "   border: 1px solid %2;"
        "   border-radius: 8px;"
        "   padding: 0 12px 0 12px;"
        "   font-size: 13px;"
        "   selection-background-color: %5;"
        "}"
        "QLineEdit:focus {"
        "   border-color: %5;"
        "}"
        "QPushButton#ChangeCoverButton {"
        "   background-color: %3;"
        "   color: %4;"
        "   border: 1px solid %2;"
        "   border-radius: 6px;"
        "   font-size: 11px;"
        "}"
        "QPushButton#ChangeCoverButton:hover {"
        "   background-color: %6;"
        "}"
        "QPushButton#SaveButton {"
        "   background-color: %5;"
        "   color: #FFFFFF;"
        "   border: none;"
        "   border-radius: 16px;"
        "   font-weight: bold;"
        "   font-size: 13px;"
        "}"
        "QPushButton#SaveButton:hover {"
        "   background-color: %7;"
        "}"
        "QPushButton#CancelButton {"
        "   background-color: transparent;"
        "   color: %8;"
        "   border: none;"
        "   font-size: 13px;"
        "}"
        "QPushButton#CancelButton:hover {"
        "   color: %4;"
        "}"
        ).arg(StyleManager::bgElevated(), StyleManager::border(),
              StyleManager::bgSurface(), StyleManager::textPrimary(),
              StyleManager::accent(), StyleManager::bgSurfaceHover(),
              StyleManager::accentHover(), StyleManager::textSecondary())
    );
}

void EditTrackDialog::refreshStyle() {
    applyQSS();

    // Re-apply cover label style
    m_coverLabel->setStyleSheet(QString("background-color: %1; border-radius: 8px; font-size: 32px; color: %2;").arg(StyleManager::bgSurface(), StyleManager::textMuted()));
}
