#ifndef FRAMELESSWINDOWHELPER_H
#define FRAMELESSWINDOWHELPER_H

#include <QObject>
#include <QPoint>
#include <QMouseEvent>
#include <QWidget>

class FramelessWindowHelper : public QObject
{
    Q_OBJECT

public:
    explicit FramelessWindowHelper(QWidget* parent)
        : QObject(parent)
        , m_parent(parent)
        , m_dragging(false)
    {}
    
    void handleMousePressEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::LeftButton) {
            m_dragging = true;
            m_dragPosition = event->globalPosition().toPoint() - m_parent->frameGeometry().topLeft();
            event->accept();
        }
    }
    
    void handleMouseMoveEvent(QMouseEvent* event)
    {
        if (m_dragging && (event->buttons() & Qt::LeftButton)) {
            m_parent->move(event->globalPosition().toPoint() - m_dragPosition);
            event->accept();
        }
    }
    
    void handleMouseReleaseEvent(QMouseEvent* event)
    {
        m_dragging = false;
        event->accept();
    }
    
    bool isDragging() const { return m_dragging; }

private:
    QWidget* m_parent;
    bool m_dragging;
    QPoint m_dragPosition;
};

#endif // FRAMELESSWINDOWHELPER_H
