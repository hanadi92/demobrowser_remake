#ifndef FIXEDELEMENT_H
#define FIXEDELEMENT_H

#include <QObject>
#include <QImage>
#include <QRectF>

class QWebEngineView;
class ScreenShotter;
class WebView;

class FixedElement : public QObject
{
    Q_OBJECT
public:
    FixedElement(WebView* parent = 0);

signals:

public slots:

public:
    friend class ScreenShotter;
    ~FixedElement();

    QImage* getImage() { return m_image; }
    QRectF getRectangle() { return m_rect; }
    QVector<QPointF> getMouseCoord() { return m_coord; }
    QString getXPath() { return m_xpath; }
    bool getIsVisible() { return isVisible; }

    void setRectangle(const QStringList &list);
    void setImage();
    void setMouseCoord(const QPointF coordinate);

    void emptyMouseCoords();
    void setXPath(const QString &xpath);
    void setIsVisible(bool visible);
private:
    QImage* m_image;
    QRectF m_rect;
    WebView* m_view;
    QVector<QPointF> m_coord;
    QString m_xpath;
    bool isVisible;
};

#endif // FIXEDELEMENT_H
