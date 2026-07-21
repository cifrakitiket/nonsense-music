#include "SettingsView.h"
#include "TranslationManager.h"
#include "StyleManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QSettings>
#include <QDebug>
#include <QMessageBox>

SettingsView::SettingsView(LibraryManager *libMgr, DownloadManager *dlMgr, QWidget *parent)
    : QWidget(parent), m_libraryManager(libMgr), m_downloadManager(dlMgr) {
    setupUI();
    applyQSS();
    
    QSettings settings("NonsenseMusic", "Player");
    bool dynBg = settings.value("dynamicBackground", true).toBool();
    m_dynamicBgCheck->setChecked(dynBg);
    
    QString dlDir = settings.value("downloadDirectory", m_downloadManager->downloadDirectory()).toString();
    m_dlDirInput->setText(dlDir);
    m_downloadManager->setDownloadDirectory(dlDir);
    
    int langIdx = static_cast<int>(TranslationManager::instance().currentLanguage());
    m_langComboBox->setCurrentIndex(langIdx);
    
    // Set initial theme selection
    int themeIdx = StyleManager::instance().currentThemeIndex();
    if (themeIdx >= 0 && themeIdx < 3) {
        m_themeBtns[themeIdx]->setChecked(true);
    }
    updateThemeButtons();
    
    connect(m_dynamicBgCheck, &QCheckBox::toggled, this, &SettingsView::onDynamicBgToggled);
    connect(m_dlDirBtn, &QPushButton::clicked, this, &SettingsView::onSelectDlDirClicked);
    connect(m_langComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &SettingsView::onLanguageChanged);
    connect(m_themeGroup, &QButtonGroup::idClicked, this, &SettingsView::onThemeClicked);
}

SettingsView::~SettingsView() {}

void SettingsView::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 12);
    mainLayout->setSpacing(12);
    mainLayout->setAlignment(Qt::AlignTop);
    
    m_titleLabel = new QLabel(trL("settings_title"), this);
    mainLayout->addWidget(m_titleLabel);
    
    // --- Section: Theme ---
    m_themeHeaderLabel = new QLabel("Тема оформления", this);
    mainLayout->addWidget(m_themeHeaderLabel);
    
    QWidget *themeRow = new QWidget(this);
    QHBoxLayout *themeLayout = new QHBoxLayout(themeRow);
    themeLayout->setContentsMargins(0, 0, 0, 0);
    themeLayout->setSpacing(10);
    themeLayout->setAlignment(Qt::AlignLeft);
    
    m_themeGroup = new QButtonGroup(this);
    m_themeGroup->setExclusive(true);
    
    // Theme dot colors for preview
    const char *dotColors[3] = { "#6C5CE7", "#D4A056", "#6B9E5A" };
    
    for (int i = 0; i < 3; ++i) {
        m_themeBtns[i] = new QPushButton(QString("  %1").arg(StyleManager::themeName(i)), this);
        m_themeBtns[i]->setCheckable(true);
        m_themeBtns[i]->setCursor(Qt::PointingHandCursor);
        m_themeBtns[i]->setMinimumHeight(34);
        m_themeBtns[i]->setMaximumHeight(42);
        m_themeBtns[i]->setMinimumWidth(160);
        m_themeBtns[i]->setObjectName(QString("ThemeBtn_%1").arg(i));
        m_themeGroup->addButton(m_themeBtns[i], i);
        themeLayout->addWidget(m_themeBtns[i]);
    }
    
    mainLayout->addWidget(themeRow);
    
    // --- Section: Dynamic Background ---
    m_dynamicBgCheck = new QCheckBox(trL("settings_dynamic_bg"), this);
    m_dynamicBgCheck->setCursor(Qt::PointingHandCursor);
    m_dynamicBgCheck->setStyleSheet(QString("color: %1; font-size: 14px; spacing: 8px;").arg(StyleManager::textPrimary()));
    mainLayout->addWidget(m_dynamicBgCheck);
    
    // --- Section: Download Location ---
    m_dlDirHeaderLabel = new QLabel(trL("settings_dl_dir_header"), this);
    mainLayout->addWidget(m_dlDirHeaderLabel);
    
    QWidget *dlDirRow = new QWidget(this);
    QHBoxLayout *dlDirLayout = new QHBoxLayout(dlDirRow);
    dlDirLayout->setContentsMargins(0, 0, 0, 0);
    dlDirLayout->setSpacing(12);
    
    m_dlDirInput = new QLineEdit(this);
    m_dlDirInput->setReadOnly(true);
    m_dlDirInput->setPlaceholderText(trL("settings_dl_dir_placeholder"));
    m_dlDirInput->setMinimumHeight(34);
    m_dlDirInput->setMaximumHeight(42);

    m_dlDirBtn = new QPushButton(trL("settings_dl_browse"), this);
    m_dlDirBtn->setMinimumHeight(34);
    m_dlDirBtn->setMaximumHeight(42);
    m_dlDirBtn->setMinimumWidth(110);
    m_dlDirBtn->setCursor(Qt::PointingHandCursor);
    
    dlDirLayout->addWidget(m_dlDirInput);
    dlDirLayout->addWidget(m_dlDirBtn);
    mainLayout->addWidget(dlDirRow);
    
    // --- Section: Language ---
    m_langHeaderLabel = new QLabel(trL("settings_lang_header"), this);
    mainLayout->addWidget(m_langHeaderLabel);
    
    m_langComboBox = new QComboBox(this);
    m_langComboBox->setMinimumHeight(34);
    m_langComboBox->setMaximumHeight(42);
    m_langComboBox->setMinimumWidth(180);
    m_langComboBox->addItems(QStringList() << "English" << "Русский" << "Українська" << "Русскій дореформенный");
    mainLayout->addWidget(m_langComboBox);
}

void SettingsView::retranslateUI() {
    m_titleLabel->setText(trL("settings_title"));
    m_dynamicBgCheck->setText(trL("settings_dynamic_bg"));
    
    m_dlDirHeaderLabel->setText(trL("settings_dl_dir_header"));
    m_dlDirInput->setPlaceholderText(trL("settings_dl_dir_placeholder"));
    m_dlDirBtn->setText(trL("settings_dl_browse"));
    
    m_langHeaderLabel->setText(trL("settings_lang_header"));
}

void SettingsView::onDynamicBgToggled(bool enabled) {
    QSettings settings("NonsenseMusic", "Player");
    settings.setValue("dynamicBackground", enabled);
    emit dynamicBackgroundToggled(enabled);
}

void SettingsView::onSelectDlDirClicked() {
    QString current = m_dlDirInput->text();
    if (current.isEmpty()) {
        current = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    }
    
    QString dir = QFileDialog::getExistingDirectory(this, "Select YouTube Download Directory", current);
    if (!dir.isEmpty()) {
        m_dlDirInput->setText(dir);
        m_downloadManager->setDownloadDirectory(dir);
        
        QSettings settings("NonsenseMusic", "Player");
        settings.setValue("downloadDirectory", dir);
    }
}

void SettingsView::onLanguageChanged(int index) {
    TranslationManager::Language lang = static_cast<TranslationManager::Language>(index);
    TranslationManager::instance().setLanguage(lang);
    emit languageChanged();
}

void SettingsView::onThemeClicked(int index) {
    StyleManager::instance().setTheme(index);
    updateThemeButtons();
    refreshStyle();
    emit themeChanged();

    QMessageBox::information(this, trL("theme_change_title"), trL("theme_change_restart_message"));
}

void SettingsView::updateThemeButtons() {
    int current = StyleManager::instance().currentThemeIndex();
    const char *dotColors[3] = { "#6C5CE7", "#D4A056", "#6B9E5A" };
    
    for (int i = 0; i < 3; ++i) {
        bool isActive = (i == current);
        m_themeBtns[i]->setChecked(isActive);
        m_themeBtns[i]->setStyleSheet(QString(
            "QPushButton {"
            "   background-color: %1;"
            "   color: %2;"
            "   border: 2px solid %3;"
            "   border-radius: 8px;"
            "   padding: 0 16px;"
            "   font-size: 13px;"
            "   font-weight: %5;"
            "   text-align: left;"
            "}"
            "QPushButton:hover {"
            "   background-color: %4;"
            "}"
            "QPushButton:checked {"
            "   border-color: %6;"
            "   background-color: %4;"
            "}"
        ).arg(
            isActive ? StyleManager::bgSurface() : StyleManager::bgSurface(),
            StyleManager::textPrimary(),
            isActive ? dotColors[i] : StyleManager::border(),
            StyleManager::bgSurfaceHover(),
            isActive ? "bold" : "normal",
            dotColors[i]
        ));
        // Prepend colored dot
        m_themeBtns[i]->setText(QString("\u25CF  %1").arg(StyleManager::themeName(i)));
    }
}



void SettingsView::refreshStyle() {
    m_titleLabel->setStyleSheet(QString("font-size: 24px; font-weight: bold; color: %1;").arg(StyleManager::textPrimary()));
    m_themeHeaderLabel->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1; margin-top: 10px;").arg(StyleManager::textPrimary()));
    m_dynamicBgCheck->setStyleSheet(QString("color: %1; font-size: 14px; spacing: 8px;").arg(StyleManager::textPrimary()));
    m_dlDirHeaderLabel->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1; margin-top: 10px;").arg(StyleManager::textPrimary()));
    m_langHeaderLabel->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1; margin-top: 10px;").arg(StyleManager::textPrimary()));
    applyQSS();
    updateThemeButtons();
}

void SettingsView::applyQSS() {
    m_dlDirInput->setStyleSheet(StyleManager::inputReadonlyStyle());
    m_dlDirBtn->setStyleSheet(StyleManager::secondaryButtonStyle());
    m_langComboBox->setStyleSheet(StyleManager::comboBoxStyle());
}
