#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include <QPainter>
#include <QRect>
#include <QColor>
#include <QPixmap>

enum class IconType {
    Play,
    Pause,
    Next,
    Previous,
    Shuffle,
    Repeat,
    RepeatOne,
    Heart,
    HeartOutline,
    Note,
    Settings,
    Account,
    Download,
    Search,
    List,
    Clock,
    VolumeOn,
    VolumeMute,
    Playlist,
    Plus,
    MiniPlayer
};

class IconProvider {
public:
    static void drawIcon(QPainter *painter, const QRect &rect, IconType type, const QColor &color);
    static QPixmap getPixmap(IconType type, const QSize &size, const QColor &color);
};

#endif // ICONPROVIDER_H
