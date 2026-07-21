#include "IconProvider.h"
#include <QPainterPath>
#include <qmath.h>

void IconProvider::drawIcon(QPainter *painter, const QRect &rect, IconType type, const QColor &color) {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    
    // Normalize coordinates so we draw on a 24x24 canvas and scale up/down
    painter->translate(rect.center());
    double scaleX = rect.width() / 24.0;
    double scaleY = rect.height() / 24.0;
    painter->scale(scaleX, scaleY);
    
    QPen pen(color);
    pen.setWidthF(1.5);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    
    QBrush brush(color);

    switch (type) {
        case IconType::Play: {
            QPainterPath path;
            path.moveTo(-4, -6);
            path.lineTo(6, 0);
            path.lineTo(-4, 6);
            path.closeSubpath();
            painter->setPen(Qt::NoPen);
            painter->setBrush(brush);
            painter->drawPath(path);
            break;
        }
        case IconType::Pause: {
            painter->setPen(Qt::NoPen);
            painter->setBrush(brush);
            painter->drawRect(-5, -6, 3, 12);
            painter->drawRect(2, -6, 3, 12);
            break;
        }
        case IconType::Next: {
            QPainterPath path;
            path.moveTo(-5, -5);
            path.lineTo(3, 0);
            path.lineTo(-5, 5);
            path.closeSubpath();
            painter->setPen(Qt::NoPen);
            painter->setBrush(brush);
            painter->drawPath(path);
            painter->drawRect(3, -5, 2, 10);
            break;
        }
        case IconType::Previous: {
            QPainterPath path;
            path.moveTo(5, -5);
            path.lineTo(-3, 0);
            path.lineTo(5, 5);
            path.closeSubpath();
            painter->setPen(Qt::NoPen);
            painter->setBrush(brush);
            painter->drawPath(path);
            painter->drawRect(-5, -5, 2, 10);
            break;
        }
        case IconType::Shuffle: {
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            QPainterPath path;
            path.moveTo(-7, 4);
            path.cubicTo(-2, 4, -2, -4, 3, -4);
            path.lineTo(7, -4);
            path.moveTo(3, -7);
            path.lineTo(7, -4);
            path.lineTo(3, -1);
            
            path.moveTo(-7, -4);
            path.cubicTo(-2, -4, -2, 4, 3, 4);
            path.lineTo(7, 4);
            path.moveTo(3, 1);
            path.lineTo(7, 4);
            path.lineTo(3, 7);
            painter->drawPath(path);
            break;
        }
        case IconType::Repeat: {
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            QPainterPath path;
            path.moveTo(-4, -5);
            path.lineTo(4, -5);
            path.arcTo(4, -5, 4, 10, 90, -180);
            path.lineTo(-4, 5);
            path.arcTo(-8, -5, 4, 10, 270, -180);
            painter->drawPath(path);
            
            QPainterPath arrow;
            arrow.moveTo(2, -8);
            arrow.lineTo(5, -5);
            arrow.lineTo(2, -2);
            painter->drawPath(arrow);
            break;
        }
        case IconType::RepeatOne: {
            drawIcon(painter, QRect(-12, -12, 24, 24), IconType::Repeat, color);
            painter->setPen(Qt::NoPen);
            painter->setBrush(color);
            QFont font = painter->font();
            font.setPixelSize(6);
            font.setBold(true);
            painter->setFont(font);
            painter->drawText(QRectF(-6, -6, 12, 12), Qt::AlignCenter, "1");
            break;
        }
        case IconType::Heart: {
            painter->setPen(Qt::NoPen);
            painter->setBrush(brush);
            QPainterPath path;
            path.moveTo(0, 3);
            path.cubicTo(0, 3, -8, -3, -8, -7);
            path.cubicTo(-8, -10, -3, -10, 0, -6);
            path.cubicTo(3, -10, 8, -10, 8, -7);
            path.cubicTo(8, -3, 0, 3, 0, 3);
            painter->drawPath(path);
            break;
        }
        case IconType::HeartOutline: {
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            QPainterPath path;
            path.moveTo(0, 3);
            path.cubicTo(0, 3, -8, -3, -8, -7);
            path.cubicTo(-8, -10, -3, -10, 0, -6);
            path.cubicTo(3, -10, 8, -10, 8, -7);
            path.cubicTo(8, -3, 0, 3, 0, 3);
            painter->drawPath(path);
            break;
        }
        case IconType::Note: {
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawLine(-2, 5, -2, -6);
            painter->drawLine(-2, -6, 5, -4);
            painter->drawLine(5, -4, 5, 3);
            painter->setPen(Qt::NoPen);
            painter->setBrush(brush);
            painter->drawEllipse(QPointF(-4, 5), 2.5, 2);
            painter->drawEllipse(QPointF(3, 3), 2.5, 2);
            break;
        }
        case IconType::Settings: {
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(QPointF(0, 0), 3, 3);
            for(int i=0; i<8; i++) {
                painter->save();
                painter->rotate(i * 45);
                painter->drawLine(0, 4, 0, 6);
                painter->restore();
            }
            break;
        }
        case IconType::Account: {
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(QPointF(0, -3), 3, 3);
            QPainterPath p;
            p.moveTo(-6, 8);
            p.cubicTo(-6, 2, 6, 2, 6, 8);
            painter->drawPath(p);
            break;
        }
        case IconType::Download: {
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawLine(0, -6, 0, 4);
            painter->drawLine(-4, 0, 0, 4);
            painter->drawLine(4, 0, 0, 4);
            painter->drawLine(-6, 7, 6, 7);
            break;
        }
        case IconType::Search: {
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(QPointF(-2, -2), 4, 4);
            painter->drawLine(1, 1, 6, 6);
            break;
        }
        case IconType::List: {
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawLine(-6, -4, 6, -4);
            painter->drawLine(-6, 0, 6, 0);
            painter->drawLine(-6, 4, 6, 4);
            break;
        }
        case IconType::Playlist: {
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawLine(-6, -4, 4, -4);
            painter->drawLine(-6, 0, 2, 0);
            painter->drawLine(-6, 4, 0, 4);
            painter->drawLine(4, -4, 4, 2);
            painter->setPen(Qt::NoPen);
            painter->setBrush(brush);
            painter->drawEllipse(QPointF(2, 2), 2, 1.5);
            break;
        }
        case IconType::Clock: {
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(QPointF(0, 0), 6, 6);
            painter->drawLine(0, -3, 0, 0);
            painter->drawLine(0, 0, 2, 2);
            break;
        }
        case IconType::VolumeOn: {
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            QPainterPath p;
            p.moveTo(-2, -2);
            p.lineTo(-5, -2);
            p.lineTo(-5, 2);
            p.lineTo(-2, 2);
            p.lineTo(2, 6);
            p.lineTo(2, -6);
            p.closeSubpath();
            painter->drawPath(p);
            painter->drawArc(3, -3, 3, 6, -60 * 16, 120 * 16);
            painter->drawArc(1, -5, 7, 10, -60 * 16, 120 * 16);
            break;
        }
        case IconType::VolumeMute: {
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            QPainterPath p;
            p.moveTo(-2, -2);
            p.lineTo(-5, -2);
            p.lineTo(-5, 2);
            p.lineTo(-2, 2);
            p.lineTo(2, 6);
            p.lineTo(2, -6);
            p.closeSubpath();
            painter->drawPath(p);
            painter->drawLine(4, -2, 8, 2);
            painter->drawLine(8, -2, 4, 2);
            break;
        }
        case IconType::Plus: {
            painter->setPen(pen);
            painter->drawLine(0, -5, 0, 5);
            painter->drawLine(-5, 0, 5, 0);
            break;
        }
        case IconType::MiniPlayer: {
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawRect(-5, -4, 10, 8);
            painter->drawLine(-5, 0, 5, 0);
            painter->drawRect(1, 1, 3, 2);
            break;
        }
    }
    
    painter->restore();
}

QPixmap IconProvider::getPixmap(IconType type, const QSize &size, const QColor &color) {
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    drawIcon(&painter, QRect(QPoint(0,0), size), type, color);
    return pixmap;
}
