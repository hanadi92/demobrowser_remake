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

#include "tabwidget.h"

#include "browserapplication.h"
#include "browsermainwindow.h"
#include "urllineedit.h"
#include "webview.h"
#include "screenshotter.h"

#include <QWebEngineProfile>
#include <QtCore/QMimeData>
#include <QtGui/QClipboard>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtGui/QDrag>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStyle>

#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QStandardPaths>
#include <QDir>
#include <QLineEdit>
#include <QVBoxLayout>

#include <QtCore/QDebug>

TabBar::TabBar(QWidget *parent)
    : QTabBar(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAcceptDrops(true);

    QString ctrl = QLatin1String("Ctrl+%1");
    for (int i = 1; i < 10; ++i) {
        QShortcut *shortCut = new QShortcut(ctrl.arg(i), this);
        m_tabShortcuts.append(shortCut);
        connect(shortCut, SIGNAL(activated()), this, SLOT(selectTabAction()));
    }
    setTabsClosable(true);
    connect(this, SIGNAL(tabCloseRequested(int)),
            this, SIGNAL(closeTab(int)));
    setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
    setMovable(true);
}

void TabBar::selectTabAction()
{
    if (QShortcut *shortCut = qobject_cast<QShortcut*>(sender())) {
        int index = m_tabShortcuts.indexOf(shortCut);
        setCurrentIndex(index);
    }
}

void TabBar::closeTab()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        int index = action->data().toInt();
        emit closeTab(index);
    }
}

void TabBar::closeOtherTabs()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        int index = action->data().toInt();
        emit closeOtherTabs(index);
    }
}

void TabBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_dragStartPos = event->pos();

    QTabBar::mousePressEvent(event);

    // Middle click on tab should close it.
    if (event->button() == Qt::MiddleButton) {
        const QPoint pos = event->pos();
        int index = tabAt(pos);
        if (index != -1) {
            emit closeTab(index);
        }
    }
}

void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        int diffX = event->pos().x() - m_dragStartPos.x();
        int diffY = event->pos().y() - m_dragStartPos.y();
        if ((event->pos() - m_dragStartPos).manhattanLength() > QApplication::startDragDistance()
            && diffX < 3 && diffX > -3
            && diffY < -10) {
            QDrag *drag = new QDrag(this);
            QMimeData *mimeData = new QMimeData;
            QList<QUrl> urls;
            int index = tabAt(event->pos());
            QUrl url = tabData(index).toUrl();
            urls.append(url);
            mimeData->setUrls(urls);
            mimeData->setText(tabText(index));
            mimeData->setData(QLatin1String("action"), "tab-reordering");
            drag->setMimeData(mimeData);
            drag->exec();
        }
    }
    QTabBar::mouseMoveEvent(event);
}

// When index is -1 index chooses the current tab
void TabWidget::reloadTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    QWidget *widget = this->widget(index);
    if (WebView *tab = qobject_cast<WebView*>(widget))
    {
        tab->deleteFixedElements();
        tab->reload();
    }
}

void TabBar::reloadTab()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        int index = action->data().toInt();
        emit reloadTab(index);
    }
}

QWebEngineProfile *TabWidget::initProfile() {

    if(m_profile != nullptr) {
        delete m_profile;
    }
    m_profile = new QWebEngineProfile("demobrowserProfile", this);
    prepairProfileJavaScript();

    QString dirName = "demobrowserData";
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    if (!dir.cd(dirName)) {
        dir.mkdir(dirName);
        dir.cd(dirName);
    }

    foreach (const QString &match, dir.entryList(QStringList("storage*"), QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot)) {
        QDir(dir.absolutePath()+ "/" + match).removeRecursively();
    }

    QString rand = "storage" + QString::number(QDateTime::currentMSecsSinceEpoch());
    dir.mkdir(rand);
    dir.cd(rand);
    m_profile->setCachePath(dir.absolutePath());
    m_profile->setPersistentStoragePath(dir.absolutePath());

    return m_profile;
}

TabWidget::TabWidget(QWidget *parent)
    : QTabWidget(parent)
    , m_recentlyClosedTabsAction(0)
    , m_newTabAction(0)
    , m_closeTabAction(0)
    , m_recentlyClosedTabsMenu(0)
    , m_lineEditCompleter(0)
    , m_lineEdits(0)
    , m_tabBar(new TabBar(this))
    , m_profile(nullptr)
    , screenshotter(new ScreenShotter(this))
    , currentUrl("")
    , Pcounter(0)
{
//    initProfile();

    setElideMode(Qt::ElideRight);

    connect(m_tabBar, SIGNAL(newTab()), this, SLOT(newTab()));
    connect(m_tabBar, SIGNAL(closeTab(int)), this, SLOT(requestCloseTab(int)));
    connect(m_tabBar, SIGNAL(closeOtherTabs(int)), this, SLOT(closeOtherTabs(int)));
    connect(m_tabBar, SIGNAL(reloadTab(int)), this, SLOT(reloadTab(int)));
    connect(m_tabBar, SIGNAL(reloadAllTabs()), this, SLOT(reloadAllTabs()));

    setTabBar(m_tabBar);
    setDocumentMode(true);

    // Actions
    m_newTabAction = new QAction(QIcon(QLatin1String(":addtab.png")), tr("New &Tab"), this);
    m_newTabAction->setShortcuts(QKeySequence::AddTab);
    m_newTabAction->setIconVisibleInMenu(false);
    connect(m_newTabAction, SIGNAL(triggered()), this, SLOT(newTab()));

    m_closeTabAction = new QAction(QIcon(QLatin1String(":closetab.png")), tr("&Close Tab"), this);
    m_closeTabAction->setShortcuts(QKeySequence::Close);
    m_closeTabAction->setIconVisibleInMenu(false);
    connect(m_closeTabAction, SIGNAL(triggered()), this, SLOT(requestCloseTab()));

    m_recentlyClosedTabsMenu = new QMenu(this);
    connect(m_recentlyClosedTabsMenu, SIGNAL(aboutToShow()),
            this, SLOT(aboutToShowRecentTabsMenu()));
    connect(m_recentlyClosedTabsMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(aboutToShowRecentTriggeredAction(QAction*)));
    m_recentlyClosedTabsAction = new QAction(tr("Recently Closed Tabs"), this);
    m_recentlyClosedTabsAction->setMenu(m_recentlyClosedTabsMenu);
    m_recentlyClosedTabsAction->setEnabled(false);

    connect(this, SIGNAL(currentChanged(int)),
            this, SLOT(currentChanged(int)));

    m_lineEdits = new QStackedWidget(this);
}

void TabWidget::setPcounter(const qint64 pc) {
    Pcounter = pc;
}


void TabWidget::setCurrentUrl(const QString &value)
{
    currentUrl = value;
}

QString TabWidget::getCurrentUrl() const
{
    return currentUrl;
}

void TabWidget::screenshotSaveImage() {
    screenshotter->saveImage(currentWebView(), getPcounter(), false);
}

void TabWidget::prepairProfileJavaScript() {
    QFile ScriptFile(":/scripts/FixedElements.js");
    if(ScriptFile.open(QIODevice::ReadOnly)) {
        QByteArray Js = ScriptFile.readAll();
        QWebEngineScript script;
        script.setSourceCode(Js);
        script.setName("FixedElements.js");
        script.setWorldId(QWebEngineScript::ApplicationWorld);
        script.setInjectionPoint(QWebEngineScript::DocumentCreation);
        script.setRunsOnSubFrames(true);
        m_profile->scripts()->insert(script);
    }
}

void TabWidget::clear()
{
    // clear the recently closed tabs
    m_recentlyClosedTabs.clear();
    // clear the line edit history
    for (int i = 0; i < m_lineEdits->count(); ++i) {
        QLineEdit *qLineEdit = lineEdit(i);
        qLineEdit->setText(qLineEdit->text());
    }
}

void TabWidget::addWebAction(QAction *action, QWebEnginePage::WebAction webAction)
{
    if (!action)
        return;
    m_actions.append(new WebActionMapper(action, webAction, this));
}

void TabWidget::currentChanged(int index)
{
    WebView *webView = this->webView(index);
    if (!webView)
        return;

    Q_ASSERT(m_lineEdits->count() == count());

    WebView *oldWebView = this->webView(m_lineEdits->currentIndex());
    if (oldWebView) {
        oldWebView->deactivate();
#if defined(QWEBENGINEVIEW_STATUSBARMESSAGE)
        disconnect(oldWebView, SIGNAL(statusBarMessage(QString)),
                this, SIGNAL(showStatusBarMessage(QString)));
#endif
        disconnect(oldWebView->page(), SIGNAL(linkHovered(const QString&)),
                this, SIGNAL(linkHovered(const QString&)));
        disconnect(oldWebView, SIGNAL(loadProgress(int)),
                this, SIGNAL(loadProgress(int)));
    }

#if defined(QWEBENGINEVIEW_STATUSBARMESSAGE)
    connect(webView, SIGNAL(statusBarMessage(QString)),
            this, SIGNAL(showStatusBarMessage(QString)));
#endif
    connect(webView->page(), SIGNAL(linkHovered(const QString&)),
            this, SIGNAL(linkHovered(const QString&)));
    connect(webView, SIGNAL(loadProgress(int)),
            this, SIGNAL(loadProgress(int)));

    for (int i = 0; i < m_actions.count(); ++i) {
        WebActionMapper *mapper = m_actions[i];
        mapper->updateCurrent(webView->page());
    }
    emit setCurrentTitle(webView->title());
    m_lineEdits->setCurrentIndex(index);
    emit loadProgress(webView->progress());
    emit showStatusBarMessage(webView->lastStatusBarText());
    if (webView->url().isEmpty()){
        m_lineEdits->currentWidget()->setFocus();
    }
    else {
        webView->activate();
        webView->setFocus();
    }
}

QAction *TabWidget::newTabAction() const
{
    return m_newTabAction;
}

QAction *TabWidget::closeTabAction() const
{
    return m_closeTabAction;
}

QAction *TabWidget::recentlyClosedTabsAction() const
{
    return m_recentlyClosedTabsAction;
}

QWidget *TabWidget::lineEditStack() const
{
    return m_lineEdits;
}

QLineEdit *TabWidget::currentLineEdit() const
{
    return lineEdit(m_lineEdits->currentIndex());
}

WebView *TabWidget::currentWebView() const
{
    return webView(currentIndex());
}

QLineEdit *TabWidget::lineEdit(int index) const
{
    UrlLineEdit *urlLineEdit = qobject_cast<UrlLineEdit*>(m_lineEdits->widget(index));
    if (urlLineEdit)
        return urlLineEdit->lineEdit();
    return 0;
}

WebView *TabWidget::webView(int index) const
{
    QWidget *widget = this->widget(index);
    if (WebView *webView = qobject_cast<WebView*>(widget)) {
        return webView;
    } else {
        // optimization to delay creating the first webview
        if (count() == 1) {
            TabWidget *that = const_cast<TabWidget*>(this);
            that->setUpdatesEnabled(false);
            that->newTab();
            that->closeTab(0);
            that->setUpdatesEnabled(true);
            return currentWebView();
        }
    }
    return 0;
}

int TabWidget::webViewIndex(WebView *webView) const
{
    int index = indexOf(webView);
    return index;
}

void TabWidget::setupPage(QWebEnginePage* page)
{
    connect(page, SIGNAL(windowCloseRequested()),
            this, SLOT(windowCloseRequested()));
    connect(page, SIGNAL(geometryChangeRequested(QRect)),
            this, SIGNAL(geometryChangeRequested(QRect)));
#if defined(QWEBENGINEPAGE_MENUBARVISIBILITYCHANGEREQUESTED)
    connect(page, SIGNAL(menuBarVisibilityChangeRequested(bool)),
            this, SIGNAL(menuBarVisibilityChangeRequested(bool)));
#endif
#if defined(QWEBENGINEPAGE_STATUSBARVISIBILITYCHANGEREQUESTED)
    connect(page, SIGNAL(statusBarVisibilityChangeRequested(bool)),
            this, SIGNAL(statusBarVisibilityChangeRequested(bool)));
#endif
#if defined(QWEBENGINEPAGE_TOOLBARVISIBILITYCHANGEREQUESTED)
    connect(page, SIGNAL(toolBarVisibilityChangeRequested(bool)),
            this, SIGNAL(toolBarVisibilityChangeRequested(bool)));
#endif

    // webview actions
    for (int i = 0; i < m_actions.count(); ++i) {
        WebActionMapper *mapper = m_actions[i];
        mapper->addChild(page->action(mapper->webAction()));
    }
}

WebView *TabWidget::newTab(bool makeCurrent)
{
    // line edit
    UrlLineEdit *urlLineEdit = new UrlLineEdit;
    QLineEdit *lineEdit = urlLineEdit->lineEdit();
    connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(lineEditReturnPressed()));
    m_lineEdits->addWidget(urlLineEdit);
    m_lineEdits->setSizePolicy(lineEdit->sizePolicy());

    // optimization to delay creating the more expensive WebView, history, etc
    if (count() == 0) {
        QWidget *emptyWidget = new QWidget;
        QPalette p = emptyWidget->palette();
        p.setColor(QPalette::Window, palette().color(QPalette::Base));
        emptyWidget->setPalette(p);
        emptyWidget->setAutoFillBackground(true);
        disconnect(this, SIGNAL(currentChanged(int)),
            this, SLOT(currentChanged(int)));
        addTab(emptyWidget, tr("(Untitled)"));
        connect(this, SIGNAL(currentChanged(int)),
            this, SLOT(currentChanged(int)));
        return 0;
    }

    // webview
    WebView *webView = new WebView(this, screenshotter);
    webView->setPage(new WebPage(m_profile, webView, screenshotter));
    urlLineEdit->setWebView(webView);
    connect(webView, SIGNAL(loadStarted()),
            this, SLOT(webViewLoadStarted()));
    connect(webView, SIGNAL(iconChanged(QIcon)),
            this, SLOT(webViewIconChanged(QIcon)));
    connect(webView, SIGNAL(titleChanged(QString)),
            this, SLOT(webViewTitleChanged(QString)));
    connect(webView, SIGNAL(urlChanged(QUrl)),
            this, SLOT(webViewUrlChanged(QUrl)));


    addTab(webView, tr("(Untitled)"));
    if (makeCurrent)
        setCurrentWidget(webView);

    setupPage(webView->page());

    if (count() == 1)
        currentChanged(currentIndex());
    emit tabsChanged();
    return webView;
}

void TabWidget::reloadAllTabs()
{
    for (int i = 0; i < count(); ++i) {
        QWidget *tabWidget = widget(i);
        if (WebView *tab = qobject_cast<WebView*>(tabWidget)) {
            tab->deleteFixedElements();
            tab->reload();
        }
    }
}

void TabWidget::lineEditReturnPressed()
{
    if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(sender())) {
        emit loadPage(lineEdit->text());
        if (m_lineEdits->currentWidget() == lineEdit)
            currentWebView()->setFocus();
    }
}

void TabWidget::windowCloseRequested()
{
    WebPage *webPage = qobject_cast<WebPage*>(sender());
    WebView *webView = qobject_cast<WebView*>(webPage->view());
    int index = webViewIndex(webView);
    if (index >= 0)
        closeTab(index);
}

void TabWidget::closeOtherTabs(int index)
{
    if (-1 == index)
        return;
    for (int i = count() - 1; i > index; --i)
        closeTab(i);
    for (int i = index - 1; i >= 0; --i)
        closeTab(i);
}

// When index is -1 index chooses the current tab
void TabWidget::requestCloseTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;
    WebView *tab = webView(index);
    if (!tab)
        return;
    tab->page()->triggerAction(QWebEnginePage::RequestClose);
}

void TabWidget::closeTab(int index)
{
    if (index < 0 || index >= count())
        return;

    bool hasFocus = false;
    if (WebView *tab = webView(index)) {
        hasFocus = tab->hasFocus();
        tab->deactivate();
    }
    QWidget *lineEdit = m_lineEdits->widget(index);
    m_lineEdits->removeWidget(lineEdit);
    lineEdit->deleteLater();
    QWidget *webView = widget(index);
    removeTab(index);
    webView->deleteLater();
    emit tabsChanged();
    if (hasFocus && count() > 0)
        currentWebView()->setFocus();
    if (count() == 0)
        emit lastTabClosed();
}

void TabWidget::setProfile(QWebEngineProfile *profile)
{
    m_profile = profile;

    for (int i = 0; i < count(); ++i) {
        QWidget *tabWidget = widget(i);
        if (WebView *tab = qobject_cast<WebView*>(tabWidget)) {
            WebPage* webPage = new WebPage(m_profile, tab, screenshotter);
            setupPage(webPage);
            webPage->load(tab->page()->url());
            tab->setPage(webPage);
        }
    }
}

void TabWidget::webViewLoadStarted()
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index) {
        QIcon icon(QLatin1String(":loading.gif"));
        setTabIcon(index, icon);
    }
}

void TabWidget::webViewIconChanged(const QIcon &icon)
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index)
        setTabIcon(index, icon);
}

void TabWidget::webViewTitleChanged(const QString &title)
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index) {
        setTabText(index, title);
    }
    if (currentIndex() == index)
        emit setCurrentTitle(title);
}

void TabWidget::webViewUrlChanged(const QUrl &url)
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index) {
        m_tabBar->setTabData(index, url);
    }
    emit tabsChanged();
}

void TabWidget::aboutToShowRecentTabsMenu()
{
    m_recentlyClosedTabsMenu->clear();
    for (int i = 0; i < m_recentlyClosedTabs.count(); ++i) {
        QAction *action = new QAction(m_recentlyClosedTabsMenu);
        action->setData(m_recentlyClosedTabs.at(i));
        QIcon icon = BrowserApplication::instance()->icon(m_recentlyClosedTabs.at(i));
        action->setIcon(icon);
        action->setText(m_recentlyClosedTabs.at(i).toString());
        m_recentlyClosedTabsMenu->addAction(action);
    }
}

void TabWidget::aboutToShowRecentTriggeredAction(QAction *action)
{
    QUrl url = action->data().toUrl();
    loadUrlInCurrentTab(url);
}

void TabWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!childAt(event->pos())
            // Remove the line below when QTabWidget does not have a one pixel frame
            && event->pos().y() < (tabBar()->y() + tabBar()->height())) {
        newTab();
        return;
    }
    QTabWidget::mouseDoubleClickEvent(event);
}

void TabWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MidButton && !childAt(event->pos())
            // Remove the line below when QTabWidget does not have a one pixel frame
            && event->pos().y() < (tabBar()->y() + tabBar()->height())) {
        QUrl url(QApplication::clipboard()->text(QClipboard::Selection));
        if (!url.isEmpty() && url.isValid() && !url.scheme().isEmpty()) {
            WebView *webView = newTab();
            webView->setUrl(url);
        }
    }
}

ScreenShotter *TabWidget::getScreenshoter() const
{
    return screenshotter;
}

void TabWidget::loadUrlInCurrentTab(const QUrl &url)
{
    WebView *webView = currentWebView();
    if (webView) {
        webView->loadUrl(url);
        webView->activate();
        webView->setFocus();
    }
}

void TabWidget::nextTab()
{
    int next = currentIndex() + 1;
    if (next == count())
        next = 0;
    setCurrentIndex(next);
}

void TabWidget::previousTab()
{
    int next = currentIndex() - 1;
    if (next < 0)
        next = count() - 1;
    setCurrentIndex(next);
}

static const qint32 TabWidgetMagic = 0xaa;

QByteArray TabWidget::saveState() const
{
    int version = 1;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << qint32(TabWidgetMagic);
    stream << qint32(version);

    QStringList tabs;
    for (int i = 0; i < count(); ++i) {
        if (WebView *tab = qobject_cast<WebView*>(widget(i))) {
            tabs.append(tab->url().toString());
        } else {
            tabs.append(QString::null);
        }
    }
    stream << tabs;
    stream << currentIndex();
    return data;
}

bool TabWidget::restoreState(const QByteArray &state)
{
    int version = 1;
    QByteArray sd = state;
    QDataStream stream(&sd, QIODevice::ReadOnly);
    if (stream.atEnd())
        return false;

    qint32 marker;
    qint32 v;
    stream >> marker;
    stream >> v;
    if (marker != TabWidgetMagic || v != version)
        return false;

    QStringList openTabs;
    stream >> openTabs;

    for (int i = 0; i < openTabs.count(); ++i) {
        if (i != 0)
            newTab();
        loadPage(openTabs.at(i));
    }

    int currentTab;
    stream >> currentTab;
    setCurrentIndex(currentTab);

    return true;
}

WebActionMapper::WebActionMapper(QAction *root, QWebEnginePage::WebAction webAction, QObject *parent)
    : QObject(parent)
    , m_currentParent(0)
    , m_root(root)
    , m_webAction(webAction)
{
    if (!m_root)
        return;
    connect(m_root, SIGNAL(triggered()), this, SLOT(rootTriggered()));
    connect(root, SIGNAL(destroyed(QObject*)), this, SLOT(rootDestroyed()));
    root->setEnabled(false);
}

void WebActionMapper::rootDestroyed()
{
    m_root = 0;
}

void WebActionMapper::currentDestroyed()
{
    updateCurrent(0);
}

void WebActionMapper::addChild(QAction *action)
{
    if (!action)
        return;
    connect(action, SIGNAL(changed()), this, SLOT(childChanged()));
}

QWebEnginePage::WebAction WebActionMapper::webAction() const
{
    return m_webAction;
}

void WebActionMapper::rootTriggered()
{
    if (m_currentParent) {
        QAction *gotoAction = m_currentParent->action(m_webAction);
        gotoAction->trigger();
    }
}

void WebActionMapper::childChanged()
{
    if (QAction *source = qobject_cast<QAction*>(sender())) {
        if (m_root
            && m_currentParent
            && source->parent() == m_currentParent) {
            m_root->setChecked(source->isChecked());
            m_root->setEnabled(source->isEnabled());
        }
    }
}

void WebActionMapper::updateCurrent(QWebEnginePage *currentParent)
{
    if (m_currentParent)
        disconnect(m_currentParent, SIGNAL(destroyed(QObject*)),
                   this, SLOT(currentDestroyed()));

    m_currentParent = currentParent;
    if (!m_root)
        return;
    if (!m_currentParent) {
        m_root->setEnabled(false);
        m_root->setChecked(false);
        return;
    }
    QAction *source = m_currentParent->action(m_webAction);
    m_root->setChecked(source->isChecked());
    m_root->setEnabled(source->isEnabled());
    connect(m_currentParent, SIGNAL(destroyed(QObject*)),
            this, SLOT(currentDestroyed()));
}
