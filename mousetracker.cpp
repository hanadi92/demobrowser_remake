#include "mousetracker.h"

MouseTracker::MouseTracker(QObject *parent)
    : QObject(parent)
    , lastMouseMoveCoord()
    , mouseMoveTimeEpsilon(30)
{

}

bool MouseTracker::isMouseMoveCoordValid(const QPointF &mouseXYPoint)
{
    if((mouseXYPoint-lastMouseMoveCoord).manhattanLength() > mouseMoveTimeEpsilon){
        lastMouseMoveCoord = mouseXYPoint;
        return true;
    }
    return false;
}
