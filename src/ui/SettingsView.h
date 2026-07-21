#ifndef SETTINGSVIEW_H
#define SETTINGSVIEW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QButtonGroup>
#include <QCheckBox>
#include "LibraryManager.h"
#include "DownloadManager.h"

class SettingsView : public QWidget {
    Q_OBJECT
public:
    explicit SettingsView(LibraryManager *libMgr, DownloadManager *dlMgr, QWidget *parent = nullptr);
    ~SettingsView();
    
    void retranslateUI();
    void refreshStyle();

signals:
    void languageChanged();
    void themeChanged();
    void dynamicBackgroundToggled(bool enabled);

private slots:
    void onSelectDlDirClicked();
    void onLanguageChanged(int index);
    void onThemeClicked(int index);
    void onDynamicBgToggled(bool enabled);

private:
    LibraryManager *m_libraryManager;
    DownloadManager *m_downloadManager;

    QLabel *m_titleLabel;
    QComboBox *m_themeCombo;
    QCheckBox *m_dynamicBgCheck;

    QLabel *m_dlDirHeaderLabel;
    QLineEdit *m_dlDirInput;
    QPushButton *m_dlDirBtn;
    
    QLabel *m_langHeaderLabel;
    QComboBox *m_langComboBox;

    QLabel *m_themeHeaderLabel;
    QButtonGroup *m_themeGroup;
    QPushButton *m_themeBtns[3];

    void setupUI();
    void applyQSS();
    void updateThemeButtons();
};

#endif // SETTINGSVIEW_H
