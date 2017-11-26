#ifndef MOUSETRACKER_H
#define MOUSETRACKER_H

#include <QObject>
#include <QPointF>

class MouseTracker : public QObject
{
    Q_OBJECT
public:
    explicit MouseTracker(QObject *parent = 0);
    bool isMouseMoveCoordValid(const QPointF &mouseXYPoint);

signals:

public slots:

private:
    qint64 mouseMoveTimeEpsilon;
    QPointF lastMouseMoveCoord;
};

#endif // MOUSETRACKER_H
