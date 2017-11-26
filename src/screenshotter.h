#ifndef SCREENSHOTTER_H
#define SCREENSHOTTER_H

#include <QObject>
#include <QDir>
#include <QPointF>
#include <QVector>
#include <QPainter>

class QWebEngineView;
class MouseTracker;
class WebView;
class QMutex;

class ScreenShotter : public QObject
{
    Q_OBJECT
public:
    explicit ScreenShotter(QObject *parent = 0);
    void addMouseMovePosition(const QPointF &mousePosition, WebView *view);
    void setScrollPosition(const QPointF &scrollPosition);
    ~ScreenShotter();
    void addFixedImages(WebView *view);
    void addMouseMoves(WebView *view);
    void addNextPart(QImage *viewImage, const QPointF &scrollPosition);
    void setLastScrollPosition(const QPointF &oldScrollPosition);
    qint32 getMaxScrollTop();
    QPoint getBigImageCoord();
    QPointF getLastScrollPosition() { return lastScrollPosition; }
    qint64 getLastImageSaveTimestamp() const { return lastImageSaveTimestamp; }
public slots:
    bool saveImage(WebView *view, qint64 pc, bool urlChange);

signals:

private slots:
    void showAlert(const QString &title, const QString &msg);

private:
    void saveImageConcurrent(const QImage& image, const QString &imageName);
    QImage* image;
    qint32 maxScrollLeft;
    qint32 maxScrollTop;
    qint64 lastPartTimestamp;
    QPointF scrollPositionForMouse;
    QDir tempDir;
    MouseTracker* mouseTracker;
    QImage* m_cursorImage;
    QVector<QPointF> all_coords;
    QMutex* imageLock;
    qint32 savedImagesCounter;
    QPointF lastScrollPosition;
    qint64 lastImageSaveTimestamp;
};

#endif // SCREENSHOTTER_H
