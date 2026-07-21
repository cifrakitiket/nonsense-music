#include "StyleManager.h"

const Theme StyleManager::s_themes[3] = {
    // Theme 0: Spotify Dark
    {
        .bgDeepest      = "#000000",
        .bgPrimary      = "#121212",
        .bgElevated     = "#181818",
        .bgSurface      = "#282828",
        .bgSurfaceHover = "#3E3E3E",
        .bgSidebar      = "#000000",
        .border         = "#3E3E3E",
        .borderLight    = "#535353",
        .accent         = "#1DB954",
        .accentHover    = "#1ed760",
        .accentDim      = "rgba(29,185,84,0.15)",
        .textPrimary    = "#FFFFFF",
        .textSecondary  = "#B3B3B3",
        .textTertiary   = "#737373",
        .textMuted      = "#535353",
        .error          = "#E91429",
        .sliderGroove   = "#535353",
        .scrollbar      = "#535353",
        .scrollbarHover = "#B3B3B3"
    },
    // Theme 1: Coffee Dark (warm brown amber)
    {
        .bgDeepest      = "#0F0A08",
        .bgPrimary      = "#181210",
        .bgElevated     = "#1E1814",
        .bgSurface      = "#261F1A",
        .bgSurfaceHover = "#302820",
        .bgSidebar      = "#120D0A",
        .border         = "#3A2E24",
        .borderLight    = "#4A3E34",
        .accent         = "#D4A056",
        .accentHover    = "#E0B066",
        .accentDim      = "rgba(212,160,86,0.15)",
        .textPrimary    = "#F5EDE6",
        .textSecondary  = "#A09080",
        .textTertiary   = "#7A6A5A",
        .textMuted      = "#5A4A3A",
        .error          = "#E07050",
        .sliderGroove   = "#3A2E24",
        .scrollbar      = "#4A3E34",
        .scrollbarHover = "#6A5E54"
    },
    // Theme 2: Moss Dark (olive green)
    {
        .bgDeepest      = "#0A0F0A",
        .bgPrimary      = "#111811",
        .bgElevated     = "#161E16",
        .bgSurface      = "#1C261C",
        .bgSurfaceHover = "#223022",
        .bgSidebar      = "#0D140D",
        .border         = "#2A362A",
        .borderLight    = "#3A483A",
        .accent         = "#6B9E5A",
        .accentHover    = "#7BAE6A",
        .accentDim      = "rgba(107,158,90,0.15)",
        .textPrimary    = "#EFF5EC",
        .textSecondary  = "#8EA088",
        .textTertiary   = "#6A7A60",
        .textMuted      = "#4A5A40",
        .error          = "#D07060",
        .sliderGroove   = "#2A362A",
        .scrollbar      = "#3A483A",
        .scrollbarHover = "#5A6A5A"
    }
};

StyleManager::StyleManager(QObject *parent) : QObject(parent) {
    QSettings settings("NonsenseMusic", "Player");
    int saved = settings.value("themeIndex", 0).toInt();
    if (saved < 0 || saved > 2) saved = 0;
    m_currentIndex = saved;
    m_current = s_themes[m_currentIndex];
}

void StyleManager::setTheme(int index) {
    if (index < 0 || index > 2) return;
    if (m_currentIndex == index) return;
    m_currentIndex = index;
    m_current = s_themes[index];

    QSettings settings("NonsenseMusic", "Player");
    settings.setValue("themeIndex", index);

    emit themeChanged();
}

QString StyleManager::themeName(int index) {
    switch (index) {
        case 0: return "Spotify Dark";
        case 1: return "Кофейная тёмная";
        case 2: return "Моховая тёмная";
        default: return "Spotify Dark";
    }
}
