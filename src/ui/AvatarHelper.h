#ifndef AVATARHELPER_H
#define AVATARHELPER_H

#include <QPixmap>
#include <QString>
#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QFontMetrics>

class AvatarHelper
{
public:
    static QPixmap drawCircularAvatar(const QPixmap& source, int size)
    {
        if (source.isNull()) {
            return QPixmap();
        }
        
        QPixmap scaled = source.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        
        QPixmap result(size, size);
        result.fill(Qt::transparent);
        
        QPainter painter(&result);
        painter.setRenderHint(QPainter::Antialiasing);
        
        QPainterPath path;
        path.addEllipse(0, 0, size, size);
        painter.setClipPath(path);
        
        int x = (size - scaled.width()) / 2;
        int y = (size - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
        
        return result;
    }
    
    static QPixmap drawDefaultAvatar(const QString& name, int size, const QColor& bgColor = QColor(0x4A, 0x90, 0xE2))
    {
        QPixmap result(size, size);
        result.fill(Qt::transparent);
        
        QPainter painter(&result);
        painter.setRenderHint(QPainter::Antialiasing);
        
        QPainterPath path;
        path.addEllipse(0, 0, size, size);
        painter.fillPath(path, bgColor);
        
        QString initial = name.isEmpty() ? "?" : QString(name[0]).toUpper();
        
        QFont font("Microsoft YaHei", size / 2, QFont::Bold);
        painter.setFont(font);
        painter.setPen(Qt::white);
        
        QFontMetrics fm(font);
        QRect textRect = fm.boundingRect(initial);
        int x = (size - textRect.width()) / 2;
        int y = (size + textRect.height()) / 2 - fm.descent();
        
        painter.drawText(x, y, initial);
        
        return result;
    }
    
private:
    AvatarHelper() = delete;
};

#endif // AVATARHELPER_H
