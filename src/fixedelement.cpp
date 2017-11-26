#include "fixedelement.h"
#include "webview.h"
#include "screenshotter.h"

#include <QDebug>
#include <QPainter>

FixedElement::FixedElement(WebView *parent)
    : QObject(parent)
    , m_image(nullptr)
    , m_rect()
    , m_view(parent)
    , m_coord()
    , m_xpath("")
    , isVisible(false)
{

}

FixedElement::~FixedElement() {
    delete m_image;
    m_coord.clear();
}

void FixedElement::setRectangle(const QStringList &list)
{
    if(list.size() == 6) {
        m_rect.setTop(list[0].toFloat());
        m_rect.setLeft(list[3].toFloat());
        m_rect.setWidth(list[4].toFloat());
        m_rect.setHeight(list[5].toFloat());
    }
//    qDebug() << "rectangle made: " << m_rect;
}

void FixedElement::setImage()
{
    if(m_image && !m_image->isNull()) {
        delete m_image;
        m_image = nullptr;
    }
    m_image = new QImage(m_rect.width(), m_rect.height(), QImage::Format_RGB32);
    QRegion imageRegion = QRegion(m_rect.x(), m_rect.y(), m_rect.width(), m_rect.height());
    if(m_rect.height() > 0) {
        QPainter painter(m_image);
        m_view->render(&painter, QPoint(0,0), imageRegion);
        painter.end();
    }
//    int number = 5;
//    int randomValue = qrand() % number;
//    m_image->save( QString::number(randomValue) + "hellp.jpg", "JPG");
//    qDebug() << "image saved " ;
}

void FixedElement::setMouseCoord(const QPointF coordinate) {
    m_coord.append(coordinate);
}

void FixedElement::emptyMouseCoords() {
    m_coord.clear();
}

void FixedElement::setXPath(const QString& xpath) {
    m_xpath = xpath;
}

void FixedElement::setIsVisible(bool visible) {
    isVisible = visible;
}
