#ifndef STYLEMANAGER_H
#define STYLEMANAGER_H

#include <QObject>
#include <QString>
#include <QSettings>
#include <QCoreApplication>
#include <QColor>

struct Theme {
    const char *bgDeepest;
    const char *bgPrimary;
    const char *bgElevated;
    const char *bgSurface;
    const char *bgSurfaceHover;
    const char *bgSidebar;

    const char *border;
    const char *borderLight;

    const char *accent;
    const char *accentHover;
    const char *accentDim;

    const char *textPrimary;
    const char *textSecondary;
    const char *textTertiary;
    const char *textMuted;

    const char *error;

    const char *sliderGroove;
    const char *scrollbar;
    const char *scrollbarHover;
};

class StyleManager : public QObject {
    Q_OBJECT
public:
    static StyleManager &instance() {
        static StyleManager inst;
        return inst;
    }

    void setTheme(int index);
    int currentThemeIndex() const { return m_currentIndex; }
    static QString themeName(int index);

    // --- Current theme accessors ---
    static const char *bgDeepest()      { return t()->bgDeepest; }
    static const char *bgPrimary()      { return t()->bgPrimary; }
    static const char *bgElevated()     { return t()->bgElevated; }
    static const char *bgSurface()      { return t()->bgSurface; }
    static const char *bgSurfaceHover() { return t()->bgSurfaceHover; }
    static const char *bgSidebar()      { return t()->bgSidebar; }
    static const char *border()         { return t()->border; }
    static const char *borderLight()    { return t()->borderLight; }
    static const char *accent()         { return t()->accent; }
    static const char *accentHover()    { return t()->accentHover; }
    static const char *accentDim()      { return t()->accentDim; }
    static const char *textPrimary()    { return t()->textPrimary; }
    static const char *textSecondary()  { return t()->textSecondary; }
    static const char *textTertiary()   { return t()->textTertiary; }
    static const char *textMuted()      { return t()->textMuted; }
    static const char *error()          { return t()->error; }
    static const char *sliderGroove()   { return t()->sliderGroove; }
    static const char *scrollbar()      { return t()->scrollbar; }
    static const char *scrollbarHover() { return t()->scrollbarHover; }

    static bool isDynamicBg() {
        return QSettings("NonsenseMusic", "Player").value("dynamicBackground", true).toBool();
    }

    static QString getDynamicBg(const char *hexColor, int alpha = 180) {
        if (!isDynamicBg()) return hexColor;
        QColor c(hexColor);
        return QString("rgba(%1, %2, %3, %4)").arg(c.red()).arg(c.green()).arg(c.blue()).arg(alpha);
    }

    // --- Style generators ---
    static QString inputStyle() {
        return QStringLiteral(
            "QLineEdit {"
            "   background-color: %1; color: %2; border: 1px solid %3;"
            "   border-radius: 8px; padding: 0 14px; font-size: 13px;"
            "   selection-background-color: %4;"
            "}"
            "QLineEdit:focus { border-color: %4; }"
        ).arg(bgSurface(), textPrimary(), border(), accent());
    }

    static QString inputReadonlyStyle() {
        return QStringLiteral(
            "QLineEdit {"
            "   background-color: %1; color: %2; border: 1px solid %3;"
            "   border-radius: 8px; padding: 0 14px; font-size: 13px;"
            "}"
        ).arg(bgSurface(), textSecondary(), border());
    }

    static QString primaryButtonStyle() {
        return QStringLiteral(
            "QPushButton { background-color: %1; color: #FFFFFF; border: none;"
            "   border-radius: 20px; font-weight: bold; font-size: 13px; padding: 0 20px; }"
            "QPushButton:hover { background-color: %2; }"
            "QPushButton:disabled { background-color: %3; color: %4; }"
        ).arg(accent(), accentHover(), bgSurface(), textTertiary());
    }

    static QString secondaryButtonStyle() {
        return QStringLiteral(
            "QPushButton { background-color: %1; color: %2; border: 1px solid %3;"
            "   border-radius: 20px; font-weight: bold; font-size: 13px; padding: 0 20px; }"
            "QPushButton:hover { background-color: %4; border-color: %5; }"
            "QPushButton:disabled { background-color: %1; color: %6; border-color: %1; }"
        ).arg(bgSurface(), textPrimary(), border(), bgSurfaceHover(), borderLight(), textMuted());
    }

    static QString subtleButtonStyle() {
        return QStringLiteral(
            "QPushButton { background-color: %1; color: %2; border: 1px solid %3;"
            "   border-radius: 16px; padding: 6px 16px; font-size: 13px; font-weight: bold; }"
            "QPushButton:hover { color: %4; background-color: %5; border-color: %6; }"
        ).arg(bgSurface(), textSecondary(), border(), textPrimary(), bgSurfaceHover(), borderLight());
    }

    static QString accentButtonSmallStyle() {
        return QStringLiteral(
            "QPushButton { background-color: %1; color: #FFFFFF; border: none;"
            "   border-radius: 16px; padding: 6px 16px; font-size: 13px; font-weight: bold; }"
            "QPushButton:hover { background-color: %2; }"
        ).arg(accent(), accentHover());
    }

    static QString scrollbarStyle() {
        return QStringLiteral(
            "QScrollBar:vertical { border: none; background: transparent; width: 8px; }"
            "QScrollBar::handle:vertical { background: %1; border-radius: 4px; min-height: 20px; }"
            "QScrollBar::handle:vertical:hover { background: %2; }"
            "QScrollBar::add-line:vertical { height: 0px; }"
            "QScrollBar::sub-line:vertical { height: 0px; }"
        ).arg(scrollbar(), scrollbarHover());
    }

    static QString sliderStyle() {
        return QStringLiteral(
            "QSlider::groove:horizontal { border: none; height: 4px; background: %1; border-radius: 2px; }"
            "QSlider::sub-page:horizontal { background: %2; border-radius: 2px; }"
            "QSlider::handle:horizontal { background: %3; width: 12px; height: 12px; margin: -4px 0; border-radius: 6px; }"
            "QSlider::handle:horizontal:hover { background: %4; width: 14px; height: 14px; margin: -5px 0; border-radius: 7px; }"
        ).arg(sliderGroove(), accent(), textPrimary(), accentHover());
    }

    static QString progressBarStyle() {
        return QStringLiteral(
            "QProgressBar { border: none; background-color: %1; height: 6px; border-radius: 3px; text-align: center; }"
            "QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %2,stop:1 %3); border-radius: 3px; }"
        ).arg(bgSurface(), accent(), accentHover());
    }

    static QString contextMenuStyle() {
        return QStringLiteral(
            "QMenu { background-color: %1; color: %2; border: 1px solid %3;"
            "   border-radius: 8px; padding: 6px; }"
            "QMenu::item { padding: 8px 24px; border-radius: 4px; }"
            "QMenu::item:selected { background-color: %4; color: #FFFFFF; }"
            "QMenu::separator { height: 1px; background-color: %3; margin: 4px 8px; }"
        ).arg(bgElevated(), textPrimary(), border(), accent());
    }

    static QString comboBoxStyle() {
        return QStringLiteral(
            "QComboBox { background-color: %1; color: %2; border: 1px solid %3;"
            "   border-radius: 8px; padding: 0 14px; font-size: 13px; }"
            "QComboBox::drop-down { border: none; }"
            "QComboBox QAbstractItemView { background-color: %1; color: %2;"
            "   selection-background-color: %4; border: 1px solid %3; border-radius: 4px; padding: 4px; }"
        ).arg(bgSurface(), textPrimary(), border(), accent());
    }

signals:
    void themeChanged();

private:
    StyleManager(QObject *parent = nullptr);
    int m_currentIndex = 0;
    Theme m_current;

    static const Theme *t() { return &instance().m_current; }
    static const Theme s_themes[3];
};

// Backward-compatible aliases (used by ~90 references across UI files)
// These map to the current theme's values
namespace SM {
    inline const char *BG_DEEPEST()      { return StyleManager::bgDeepest(); }
    inline const char *BG_PRIMARY()      { return StyleManager::bgPrimary(); }
    inline const char *BG_ELEVATED()     { return StyleManager::bgElevated(); }
    inline const char *BG_SURFACE()      { return StyleManager::bgSurface(); }
    inline const char *BG_SURFACE_HOVER(){ return StyleManager::bgSurfaceHover(); }
    inline const char *BG_SIDEBAR()      { return StyleManager::bgSidebar(); }
    inline const char *BORDER()          { return StyleManager::border(); }
    inline const char *BORDER_LIGHT()    { return StyleManager::borderLight(); }
    inline const char *ACCENT()          { return StyleManager::accent(); }
    inline const char *ACCENT_HOVER()    { return StyleManager::accentHover(); }
    inline const char *ACCENT_DIM()      { return StyleManager::accentDim(); }
    inline const char *TEXT_PRIMARY()    { return StyleManager::textPrimary(); }
    inline const char *TEXT_SECONDARY()  { return StyleManager::textSecondary(); }
    inline const char *TEXT_TERTIARY()   { return StyleManager::textTertiary(); }
    inline const char *TEXT_MUTED()      { return StyleManager::textMuted(); }
    inline const char *ERROR()           { return StyleManager::error(); }
    inline const char *SLIDER_GROOVE()   { return StyleManager::sliderGroove(); }
    inline const char *SCROLLBAR()       { return StyleManager::scrollbar(); }
    inline const char *SCROLLBAR_HOVER() { return StyleManager::scrollbarHover(); }
}

#endif // STYLEMANAGER_H
