/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "browserapplication.h"
#include "browsermainwindow.h"
#include "ui_passworddialog.h"
#include "ui_proxy.h"
#include "tabwidget.h"
#include "webview.h"
#include "screenshotter.h"
#include "fixedelement.h"

#include <QtGui/QClipboard>
#include <QtNetwork/QAuthenticator>
#include <QtNetwork/QNetworkReply>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtGui/QMouseEvent>
#include <QWebEngineScript>
#include <QWebEngineContextMenuData>
#include <QWebEngineCertificateError>

#include <QStyle>

#ifndef QT_NO_UITOOLS
#include <QtUiTools/QUiLoader>
#endif  //QT_NO_UITOOLS

#include <QtCore/QDebug>
#include <QtCore/QBuffer>
#include <QtCore/QTimer>

WebPage::WebPage(QWebEngineProfile *profile, WebView *parent, ScreenShotter *screenshotter)
    : QWebEnginePage(profile, parent)
    , screenshotter(screenshotter)
    , m_view(parent)
    , m_keyboardModifiers(Qt::NoModifier)
    , m_pressedButtons(Qt::NoButton)
{
#if defined(QWEBENGINEPAGE_SETNETWORKACCESSMANAGER)
    setNetworkAccessManager(BrowserApplication::networkAccessManager());
#endif
#if defined(QWEBENGINEPAGE_UNSUPPORTEDCONTENT)
    connect(this, SIGNAL(unsupportedContent(QNetworkReply*)),
            this, SLOT(handleUnsupportedContent(QNetworkReply*)));
#endif
    connect(this, SIGNAL(authenticationRequired(const QUrl &, QAuthenticator*)),
            SLOT(authenticationRequired(const QUrl &, QAuthenticator*)));
    connect(this, SIGNAL(proxyAuthenticationRequired(const QUrl &, QAuthenticator *, const QString &)),
            SLOT(proxyAuthenticationRequired(const QUrl &, QAuthenticator *, const QString &)));
}

BrowserMainWindow *WebPage::mainWindow()
{
    QObject *w = this->parent();
    while (w) {
        if (BrowserMainWindow *mw = qobject_cast<BrowserMainWindow*>(w))
            return mw;
        w = w->parent();
    }
    return BrowserApplication::instance()->mainWindow();
}

bool WebPage::certificateError(const QWebEngineCertificateError &error)
{
    if (error.isOverridable()) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(error.errorDescription());
        msgBox.setInformativeText(tr("If you wish so, you may continue with an unverified certificate. "
                                     "Accepting an unverified certificate means "
                                     "you may not be connected with the host you tried to connect to.\n"
                                     "Do you wish to override the security check and continue?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        return msgBox.exec() == QMessageBox::Yes;
    }
    QMessageBox::critical(view(), tr("Certificate Error"), error.errorDescription(), QMessageBox::Ok, QMessageBox::NoButton);
    return false;
}

class PopupWindow : public TabWidget {
    Q_OBJECT
public:
    PopupWindow(QWebEngineProfile *profile, ScreenShotter *screenshotter)
        : m_addressBar(new QLineEdit(this))
        , m_view(new WebView(this, screenshotter))
    {
        m_view->setPage(new WebPage(profile, m_view, screenshotter));
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);
        setLayout(layout);
        layout->addWidget(m_addressBar);
        layout->addWidget(m_view);
        m_view->setFocus();

        connect(m_view, &WebView::titleChanged, this, &QWidget::setWindowTitle);
        connect(m_view, &WebView::urlChanged, this, &PopupWindow::setUrl);
        connect(page(), &WebPage::geometryChangeRequested, this, &PopupWindow::adjustGeometry);
        connect(page(), &WebPage::windowCloseRequested, this, &QWidget::close);
    }

    QWebEnginePage* page() const { return m_view->page(); }

private Q_SLOTS:
    void setUrl(const QUrl &url)
    {
        m_addressBar->setText(url.toString());
    }

    void adjustGeometry(const QRect &newGeometry)
    {
        const int x1 = frameGeometry().left() - geometry().left();
        const int y1 = frameGeometry().top() - geometry().top();
        const int x2 = frameGeometry().right() - geometry().right();
        const int y2 = frameGeometry().bottom() - geometry().bottom();

        setGeometry(newGeometry.adjusted(x1, y1 - m_addressBar->height(), x2, y2));
    }

private:
    QLineEdit *m_addressBar;
    WebView *m_view;
};

#include "webview.moc"

QWebEnginePage *WebPage::createWindow(QWebEnginePage::WebWindowType type)
{
    if (type == QWebEnginePage::WebBrowserTab) {
        return mainWindow()->tabWidget()->newTab()->page();
    } else if (type == QWebEnginePage::WebBrowserBackgroundTab) {
        return mainWindow()->tabWidget()->newTab(false)->page();
    } else if (type == QWebEnginePage::WebBrowserWindow) {
        BrowserApplication::instance()->newMainWindow();
        BrowserMainWindow *mainWindow = BrowserApplication::instance()->mainWindow();
        return mainWindow->currentTab()->page();
    } else {
        PopupWindow *popup = new PopupWindow(profile(), screenshotter);
        popup->setAttribute(Qt::WA_DeleteOnClose);
        popup->show();
        return popup->page();
    }
}

bool WebPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    if(isMainFrame) {
        if(type == QWebEnginePage::NavigationTypeLinkClicked) {
            return m_view->navigationRequest(url);
        } else {
            return m_view->navigationRequest(url);
        }
    } else {
        return true;
    }
}

#if !defined(QT_NO_UITOOLS)
QObject *WebPage::createPlugin(const QString &classId, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues)
{
    Q_UNUSED(url);
    Q_UNUSED(paramNames);
    Q_UNUSED(paramValues);
    QUiLoader loader;
    return loader.createWidget(classId, view());
}
#endif // !defined(QT_NO_UITOOLS)

#if defined(QWEBENGINEPAGE_UNSUPPORTEDCONTENT)
void WebPage::handleUnsupportedContent(QNetworkReply *reply)
{
    QString errorString = reply->errorString();

    if (m_loadingUrl != reply->url()) {
        // sub resource of this page
        qWarning() << "Resource" << reply->url().toEncoded() << "has unknown Content-Type, will be ignored.";
        reply->deleteLater();
        return;
    }

    if (reply->error() == QNetworkReply::NoError && !reply->header(QNetworkRequest::ContentTypeHeader).isValid()) {
        errorString = "Unknown Content-Type";
    }

    QFile file(QLatin1String(":/notfound.html"));
    bool isOpened = file.open(QIODevice::ReadOnly);
    Q_ASSERT(isOpened);
    Q_UNUSED(isOpened)

    QString title = tr("Error loading page: %1").arg(reply->url().toString());
    QString html = QString(QLatin1String(file.readAll()))
                        .arg(title)
                        .arg(errorString)
                        .arg(reply->url().toString());

    QBuffer imageBuffer;
    imageBuffer.open(QBuffer::ReadWrite);
    QIcon icon = view()->style()->standardIcon(QStyle::SP_MessageBoxWarning, 0, view());
    QPixmap pixmap = icon.pixmap(QSize(32,32));
    if (pixmap.save(&imageBuffer, "PNG")) {
        html.replace(QLatin1String("IMAGE_BINARY_DATA_HERE"),
                     QString(QLatin1String(imageBuffer.buffer().toBase64())));
    }

    QList<QWebEngineFrame*> frames;
    frames.append(mainFrame());
    while (!frames.isEmpty()) {
        QWebEngineFrame *frame = frames.takeFirst();
        if (frame->url() == reply->url()) {
            frame->setHtml(html, reply->url());
            return;
        }
        QList<QWebEngineFrame *> children = frame->childFrames();
        foreach (QWebEngineFrame *frame, children)
            frames.append(frame);
    }
    if (m_loadingUrl == reply->url()) {
        mainFrame()->setHtml(html, reply->url());
    }
}
#endif

void WebPage::authenticationRequired(const QUrl &requestUrl, QAuthenticator *auth)
{
    BrowserMainWindow *mainWindow = BrowserApplication::instance()->mainWindow();

    QDialog dialog(mainWindow);
    dialog.setWindowFlags(Qt::Sheet);

    Ui::PasswordDialog passwordDialog;
    passwordDialog.setupUi(&dialog);

    passwordDialog.iconLabel->setText(QString());
    passwordDialog.iconLabel->setPixmap(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow).pixmap(32, 32));

    QString introMessage = tr("<qt>Enter username and password for \"%1\" at %2</qt>");
    introMessage = introMessage.arg(auth->realm()).arg(requestUrl.toString().toHtmlEscaped());
    passwordDialog.introLabel->setText(introMessage);
    passwordDialog.introLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted) {
        auth->setUser(passwordDialog.userNameLineEdit->text());
        auth->setPassword(passwordDialog.passwordLineEdit->text());
    } else {
        // Set authenticator null if dialog is cancelled
        *auth = QAuthenticator();
    }
}

void WebPage::proxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth, const QString &proxyHost) {
    Q_UNUSED(requestUrl);
    BrowserMainWindow *mainWindow = BrowserApplication::instance()->mainWindow();

    QDialog dialog(mainWindow);
    dialog.setWindowFlags(Qt::Sheet);

    Ui::ProxyDialog proxyDialog;
    proxyDialog.setupUi(&dialog);

    proxyDialog.iconLabel->setText(QString());
    proxyDialog.iconLabel->setPixmap(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow).pixmap(32, 32));

    QString introMessage = tr("<qt>Connect to proxy \"%1\" using:</qt>");
    introMessage = introMessage.arg(proxyHost.toHtmlEscaped());
    proxyDialog.introLabel->setText(introMessage);
    proxyDialog.introLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted) {
        auth->setUser(proxyDialog.userNameLineEdit->text());
        auth->setPassword(proxyDialog.passwordLineEdit->text());
    } else {
        // Set authenticator null if dialog is cancelled
        *auth = QAuthenticator();
    }
}

void WebPage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID) {
    Q_UNUSED(level);
    Q_UNUSED(lineNumber);
    Q_UNUSED(sourceID);
    if(message.startsWith("demobrowser_")) {
        QStringList list = message.split(" ");
        QString key = list[0].mid(12);
        if(list.size() == 3) {
            QPoint coord = QPoint(list[1].toInt(), list[2].toInt());
            if(key == "move") {
                coord = m_view->mapFromGlobal(coord);
                m_view->mouseMoveDetected(coord);
            }
        } else if(key == "fixedelements") {
            QVariant fxdelemnts = list[1];
            m_view->createFixedElements(fxdelemnts);
        }
    }
}

#include <QtConcurrent/QtConcurrentRun>

WebView::WebView(TabWidget *parent, ScreenShotter *screenshotter)
    : QWebEngineView(parent)
    , m_progress(0)
    , m_page(0)
    , checkScrollPositionTimer(new QTimer(this))
    , screenshotter(screenshotter)
    , m_fixedElements(new QVector<FixedElement*>)
    , firstTimeActive(true)
    , parentTabWidget(parent)
    , scrollPositionTime(200)
{
    connect(this, SIGNAL(loadProgress(int)),
            this, SLOT(setProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)),
            this, SLOT(loadFinished(bool)));
    connect(this, &QWebEngineView::renderProcessTerminated,
            [=](QWebEnginePage::RenderProcessTerminationStatus termStatus, int statusCode) {
        const char *status = "";
        switch (termStatus) {
        case QWebEnginePage::NormalTerminationStatus:
            status = "(normal exit)";
            break;
        case QWebEnginePage::AbnormalTerminationStatus:
            status = "(abnormal exit)";
            break;
        case QWebEnginePage::CrashedTerminationStatus:
            status = "(crashed)";
            break;
        case QWebEnginePage::KilledTerminationStatus:
            status = "(killed)";
            break;
        }

        qInfo() << "Render process exited with code" << statusCode << status;
        QTimer::singleShot(0, [this] { reload(); });
    });
    connect(checkScrollPositionTimer, SIGNAL(timeout()), this, SLOT(checkScrollPositionTimeout()));
}

void WebView::checkScrollPositionTimeout() {
    if(!page()->url().isEmpty()){
        checkScrollPosition();
    }
}

void WebView::checkScrollPosition() {
    qint64 ts = QDateTime::currentMSecsSinceEpoch();
    if(!page()->url().isEmpty() && (ts - screenshotter->getLastImageSaveTimestamp()) > 500) {

        page()->runJavaScript("var x = '';\
                              if(typeof getFixedElements == 'function') {\
                                    x = getFixedElements(1);\
                               } [x];",
                              QWebEngineScript::ApplicationWorld,
                              [this](const QVariant& info) {
            if(info.isValid() && !info.isNull()) {
                    updateScreenshot(info);


            }
        });
    }
}

void WebView::updateScreenshot(const QVariant &info) {
    QList<QVariant> list = info.toList();
    if(list.length() == 1) {
        QPointF scrollPosition = page()->scrollPosition();

        screenshotter->setScrollPosition(scrollPosition);

        QPointF lastScrollPosition = screenshotter->getLastScrollPosition();
        QPoint bigImageCoord = screenshotter->getBigImageCoord();

        if(scrollPosition == lastScrollPosition
                || (scrollPosition.y() > lastScrollPosition.y() && scrollPosition.y() >= bigImageCoord.y() - 100)
                || (scrollPosition.x() > lastScrollPosition.x() && scrollPosition.x() >= bigImageCoord.x() - 100)) {

            if(list[0].isValid() && list[0] != "")
            {
                createFixedElements(list[0]);
            }

            // create an image of the viewport
            QImage* image = new QImage(width(), height(), QImage::Format_ARGB32);
            QPainter painter(image);
            image->fill(Qt::transparent);
            QRegion region = QRegion(image->rect());
            for(FixedElement *item: *m_fixedElements) {
                if(item->getIsVisible()) {
                    QRectF rect = item->getRectangle();
                    region = region - QRegion(rect.x(), rect.y(), rect.width(), rect.height());
                }
            }
            painter.setClipRegion(region);
            render(&painter);
            painter.end();

            // run add next part
            QtConcurrent::run(screenshotter, &ScreenShotter::addNextPart, image, page()->scrollPosition());
        }

        screenshotter->setLastScrollPosition(scrollPosition);
    }
}

WebView::~WebView() {
    deleteFixedElements();
    delete m_fixedElements;
}

void WebView::createFixedElements(const QVariant &fxdelemnts) {
    QString list = fxdelemnts.toString();
    QStringList elementInfo = list.split("|", QString::SkipEmptyParts);
    int numOfElements = elementInfo.size();

    // for enabling disabling items
    QVector<QString> availableXPaths;
    QVector<QString> incomingXPaths;

    for(int x = 0; x < numOfElements; x++) {

        QStringList info = elementInfo[x].split(",");

        if(info.length() == 7) {
            bool oldElement = false;

            // check if element already exists
            FixedElement* item = getFixedElementOfXPath(info[6]);
            if(item != nullptr) {
                oldElement = true;
                item->setIsVisible(true);
                item->setRectangle(info.mid(0,6));
                item->setImage();
            }

            incomingXPaths.append(info[6]);

            if(!oldElement) {
                FixedElement* el = new FixedElement(this);
                el->setIsVisible(true);
                el->setXPath(info[6]);
                el->setRectangle(info.mid(0,6));
                el->setImage();
                m_fixedElements->append(el);
            }
        }
    }

    for(FixedElement* item: *m_fixedElements) {
        QString xp = item->getXPath();
        availableXPaths.append(xp);
    }
    // disable items with xpaths that don't appear anymore
    for(QString availableXPath: availableXPaths) {
        if(!incomingXPaths.contains(availableXPath)) {
            FixedElement* item = getFixedElementOfXPath(availableXPath);
            if(item != nullptr) {
                item->setIsVisible(false);
            }
        }
    }
}

FixedElement* WebView::getFixedElementOfXPath(const QString &xpath) {
    for(FixedElement* item: *m_fixedElements) {
        QString xp = item->getXPath();
        if(xp == xpath) {
            return item;
        }
    }
    return nullptr;
}

void WebView::mouseMoveDetected(const QPoint& move) {
    screenshotter->addMouseMovePosition(move, this);
}

void WebView::deleteFixedElements() {
    for(FixedElement *item: *m_fixedElements) {
        delete item;
    }
    m_fixedElements->clear();
}

void WebView::activate() {
    checkScrollPositionTimer->start(scrollPositionTime);
    parentTabWidget->setCurrentUrl(url().toString());
    firstTimeActive = false;
    parentTabWidget->setPcounter(parentTabWidget->getPcounter()+1);
}

void WebView::deactivate() {
    if(checkScrollPositionTimer->isActive()) {
        checkScrollPositionTimer->stop();
        if(!page()->url().isEmpty()) {
            screenshotter->saveImage(this, parentTabWidget->getPcounter(), true);
        }
    }
}

void WebView::setPage(WebPage *_page)
{
    if (m_page)
        m_page->deleteLater();
    m_page = _page;
    QWebEngineView::setPage(_page);
#if defined(QWEBENGINEPAGE_STATUSBARMESSAGE)
    connect(page(), SIGNAL(statusBarMessage(QString)),
            SLOT(setStatusBarText(QString)));
#endif
    disconnect(page(), &QWebEnginePage::iconChanged, this, &WebView::iconChanged);
    connect(page(), SIGNAL(iconChanged(QIcon)),
            this, SLOT(onIconChanged(QIcon)));
#if defined(QWEBENGINEPAGE_UNSUPPORTEDCONTENT)
    page()->setForwardUnsupportedContent(true);
#endif
}

bool WebView::navigationRequest(const QUrl &newUrl)
{
    QString newURL = newUrl.toString();
    QString oldUrlString = parentTabWidget->getCurrentUrl();
    if(firstTimeActive || (!newUrl.isEmpty())) {
        handleUrlChange(oldUrlString, newURL);
    }
    return true;
}

void WebView::handleUrlChange(const QString &oldUrlString, const QString &newUrlString) {
    if(!firstTimeActive && oldUrlString != newUrlString) {
        if(checkScrollPositionTimer->isActive())
            checkScrollPositionTimer->stop();

        parentTabWidget->setCurrentUrl(newUrlString);

        bool isSaved = screenshotter->saveImage(this, parentTabWidget->getPcounter(), true);
        if(isSaved) {
            parentTabWidget->setPcounter(parentTabWidget->getPcounter());
        }

        if(!checkScrollPositionTimer->isActive())
            checkScrollPositionTimer->start(scrollPositionTime);
    }
}

void WebView::wheelEvent(QWheelEvent *event)
{
#if defined(QWEBENGINEPAGE_SETTEXTSIZEMULTIPLIER)
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 15;
        setTextSizeMultiplier(textSizeMultiplier() + numSteps * 0.1);
        event->accept();
        return;
    }
#endif
    QWebEngineView::wheelEvent(event);
}

void WebView::setProgress(int progress)
{
    m_progress = progress;
}

void WebView::loadFinished(bool success)
{
    if (success && 100 != m_progress) {
        qWarning() << "Received finished signal while progress is still:" << progress()
                   << "Url:" << url();
    }
    m_progress = 0;
}

void WebView::loadUrl(const QUrl &url)
{
    m_initialUrl = url;
    load(url);
}

QString WebView::lastStatusBarText() const
{
    return m_statusBarText;
}

QUrl WebView::url() const
{
    QUrl url = QWebEngineView::url();
    if (!url.isEmpty())
        return url;

    return m_initialUrl;
}

void WebView::onIconChanged(const QIcon &icon)
{
    if (icon.isNull())
        emit iconChanged(BrowserApplication::instance()->defaultIcon());
    else
        emit iconChanged(icon);
}

void WebView::mousePressEvent(QMouseEvent *event)
{
    m_page->m_pressedButtons = event->buttons();
    m_page->m_keyboardModifiers = event->modifiers();
    QWebEngineView::mousePressEvent(event);
}

void WebView::mouseReleaseEvent(QMouseEvent *event)
{
    QWebEngineView::mouseReleaseEvent(event);
    if (!event->isAccepted() && (m_page->m_pressedButtons & Qt::MidButton)) {
        QUrl url(QApplication::clipboard()->text(QClipboard::Selection));
        if (!url.isEmpty() && url.isValid() && !url.scheme().isEmpty()) {
            setUrl(url);
        }
    }
}

void WebView::setStatusBarText(const QString &string)
{
    m_statusBarText = string;
}
