#include "screenshotter.h"
#include "webview.h"
#include "fixedelement.h"
#include "mousetracker.h"

#include <QPainter>
#include <QMessageBox>
#include <QMutex>
#include <QThread>
#include <QtConcurrent/QtConcurrentRun>

ScreenShotter::ScreenShotter(QObject *parent)
    : QObject(parent)
    , image(nullptr)
    , maxScrollLeft(0)
    , maxScrollTop(0)
    , lastPartTimestamp(0)
    , scrollPositionForMouse(-1,-1)
    , tempDir(QDir::homePath() + "/Downloads/")
    , mouseTracker(new MouseTracker(this))
    , m_cursorImage(nullptr)
    , all_coords()
    , imageLock(new QMutex())
    , savedImagesCounter(0)
    , lastScrollPosition(0,0)
    , lastImageSaveTimestamp(0)
{

}

ScreenShotter::~ScreenShotter() {
    all_coords.clear();
    delete m_cursorImage;
    delete image;
}

void ScreenShotter::addNextPart(QImage* viewImage, const QPointF& scrollPosition)
{
    imageLock->lock();
    qint64 cts = QDateTime::currentMSecsSinceEpoch();
    if(cts - lastPartTimestamp > 200 ) {
        lastPartTimestamp = cts;

        int width = viewImage->width();
        int height = viewImage->height();
        int scrollLeft = scrollPosition.x();
        int scrollTop = scrollPosition.y();
        maxScrollLeft = std::max(maxScrollLeft, scrollLeft);
        maxScrollTop = std::max(maxScrollTop, scrollTop);

        QImage* biggerImage = new QImage(maxScrollLeft+width, maxScrollTop+height, QImage::Format_RGB32);
        QPainter painter(biggerImage);

        if(image == nullptr) {
            image = new QImage(scrollLeft+width, scrollTop+height, QImage::Format_RGB32);
        }

        painter.drawImage(0,0,*image);
        painter.drawImage(scrollLeft, scrollTop, *viewImage);

        delete viewImage;
        delete image;
        image = biggerImage;

        painter.end();
    } else {
        delete viewImage;
    }
    imageLock->unlock();
}

void ScreenShotter::setLastScrollPosition(const QPointF &oldScrollPosition) {
    lastScrollPosition = oldScrollPosition;
}

void ScreenShotter::addFixedImages(WebView* view) {
    int height = view->height();
    QPainter p(image);

    for(FixedElement* item: *view->getFixedElements()) {
        QImage* fxdImage = item->getImage();
        QRectF fxdRect = item->getRectangle();

        if(fxdRect.bottom() > height/2) {
            if(maxScrollTop != 0) {
                fxdRect.setY(maxScrollTop - (fxdRect.height() - fxdRect.bottom()));
            }
        }
        p.drawImage(fxdRect, *fxdImage);
    }
    p.end();
}

void ScreenShotter::addMouseMoves(WebView *view)
{
    int height = view->height();

    QString cursorfile = ":arrow.svg";
    QImage* cursor = new QImage(cursorfile);
    QImage cursorSmall = cursor->scaled(15,15);
    delete cursor;

    m_cursorImage = new QImage(image->width(), image->height(), QImage::Format_ARGB32);
    QPainter p1(m_cursorImage);
    m_cursorImage->fill(Qt::transparent);

    for(QPointF coord: all_coords) {
        p1.drawImage(coord, cursorSmall);
    }

    for(FixedElement* item: *view->getFixedElements()) {
        QRectF fxdRect = item->getRectangle();
        QVector<QPointF> coords = item->getMouseCoord();

        if(fxdRect.bottom() > height/2) {
            int y = fxdRect.y();
            if(maxScrollTop != 0) {
                y = maxScrollTop - (fxdRect.height() - fxdRect.bottom());
            }
            for(QPointF point: coords) {
                if(maxScrollTop != 0) {
                    point.setY(y + (point.y() - fxdRect.top()));
                }
                p1.drawImage(point, cursorSmall);
            }
        } else {
            for(QPointF point: coords) {
                p1.drawImage(point, cursorSmall);
            }
        }
        item->emptyMouseCoords();
    }
    p1.end();
}

bool ScreenShotter::saveImage(WebView* view, qint64 pc, bool urlChange)
{
    imageLock->lock();
    bool imageSaved = false;

    if(image && !image->isNull()) {
        lastImageSaveTimestamp = QDateTime::currentMSecsSinceEpoch();

        savedImagesCounter = savedImagesCounter + 1;
        QString imageName = QString::number(savedImagesCounter) + "_" + "demobrowser_image_" + QString::number(pc) + ".png";
        QString cimageName = QString::number(savedImagesCounter) + "_" + "demobrowser_cimage_" + QString::number(pc) + ".png";

        addFixedImages(view);
        addMouseMoves(view);
        saveImageConcurrent(*image, imageName);
        if(m_cursorImage && !m_cursorImage->isNull()) {
            saveImageConcurrent(*m_cursorImage, cimageName);
        }
        imageSaved = true;
        if(urlChange) {
            view->deleteFixedElements();
            delete image;
            image = nullptr;
            delete m_cursorImage;
            m_cursorImage = nullptr;
            all_coords.clear();
            maxScrollLeft = 0;
            maxScrollTop = 0;
            lastScrollPosition = QPoint(0,0);
        }
    }
    imageLock->unlock();
    return imageSaved;
}

qint32 ScreenShotter::getMaxScrollTop() {
    return maxScrollTop;
}

QPoint ScreenShotter::getBigImageCoord()
{
    if(image != nullptr) {
        return QPoint(image->width(), image->height());
    } else {
        return QPoint(0,0);
    }
}

void ScreenShotter::saveImageConcurrent(const QImage& image, const QString &imageName)
{
    QString dirName = "demobrowser";
    if(!tempDir.cd(dirName)) {
        tempDir.mkdir(dirName);
        tempDir.cd(dirName);
    }
    QString fileName = tempDir.filePath(imageName);
    if(!image.save(fileName, "PNG")) {
        showAlert(tr("Error while writing file to local disk"), imageName);
    }
    tempDir.cdUp();
}

void ScreenShotter::showAlert(const QString& title, const QString& msg) {
    QMessageBox msgBox;
    msgBox.setText(title);
    msgBox.setDetailedText(msg);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::NoButton);
    msgBox.exec();
}

void ScreenShotter::setScrollPosition(const QPointF &scrollPosition) {
    if(scrollPosition != this->scrollPositionForMouse) {
        this->scrollPositionForMouse = scrollPosition;
    }
}

void ScreenShotter::addMouseMovePosition(const QPointF &mousePosition, WebView *view) {
    QPointF coordinate = mousePosition; // + scrollPosition;
    if(mouseTracker->isMouseMoveCoordValid(coordinate)){
        for(FixedElement* item: *view->getFixedElements()) {
            bool isVisible = item->getIsVisible();
            QRectF fxdRect = item->getRectangle();
            if(fxdRect.contains(coordinate) && isVisible) {
                item->setMouseCoord(coordinate);
            } else {
                all_coords.append(coordinate + scrollPositionForMouse);
            }
        }
    }
}
