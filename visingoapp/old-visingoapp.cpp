/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "visingoapp.h"

#include "browsermainwindow.h"
#include "tabwidget.h"
#include "webview.h"

#include <QtCore/QBuffer>
#include <QtCore/QDir>
#include <QtCore/QLibraryInfo>
#include <QtCore/QSettings>
#include <QtCore/QTextStream>

#include <QtGui/QDesktopServices>
#include <QtGui/QFileOpenEvent>
#include <QtWidgets/QMessageBox>

#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

#include <QtCore/QDebug>

VisingoApp::VisingoApp(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_localServer(0)
{
    QCoreApplication::setOrganizationName(QLatin1String("Aalto University Department of Computer Science, Computational Logic Group"));
    QCoreApplication::setApplicationName(QLatin1String("visingo"));
    QCoreApplication::setApplicationVersion(QLatin1String("0.1"));
    QString serverName = QCoreApplication::applicationName()
        + QString::fromLatin1(QT_VERSION_STR).remove('.') + QLatin1String("webengine");

    // IPC, if server is already running, return
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (socket.waitForConnected(500)) {
        QTextStream stream(&socket);
        QStringList args = QCoreApplication::arguments();
        if (args.count() > 1)
            stream << args.last();
        else
            stream << QString();
        stream.flush();
        socket.waitForBytesWritten();
        return;
    }

#if defined(Q_OS_OSX)
    QApplication::setQuitOnLastWindowClosed(false);
#else
    QApplication::setQuitOnLastWindowClosed(true);
#endif

    // IPC
    m_localServer = new QLocalServer(this);
    connect(m_localServer, SIGNAL(newConnection()),
            this, SLOT(newLocalSocketConnection()));
    if (!m_localServer->listen(serverName)) {
        if (m_localServer->serverError() == QAbstractSocket::AddressInUseError
            && QFile::exists(m_localServer->serverName())) {
            QFile::remove(m_localServer->serverName());
            m_localServer->listen(serverName);
        }
    }

    // TODO remove unnecessary below
    //QDesktopServices::setUrlHandler(QLatin1String("http"), this, "openUrl");

#if defined(Q_OS_OSX)
    connect(this, SIGNAL(lastWindowClosed()),
            this, SLOT(lastWindowClosed()));
#endif

    // Delay initializing the rest, do it post launch
    QTimer::singleShot(0, this, SLOT(postLaunch()));
}

VisingoApp::~VisingoApp()
{
    for (int i = 0; i < m_mainWindows.size(); ++i) {
        BrowserMainWindow *window = m_mainWindows.at(i);
        delete window;
    }
}

#if defined(Q_OS_OSX)
void VisingoApp::lastWindowClosed()
{
    // TODO: clean up clingo related stuff also

    clean();
    BrowserMainWindow *mw = new BrowserMainWindow;
    mw->slotHome();
    m_mainWindows.prepend(mw);
}
#endif

VisingoApp *VisingoApp::instance()
{
    return (static_cast<VisingoApp *>(QCoreApplication::instance()));
}

#if defined(Q_OS_OSX)
#include <QtWidgets/QMessageBox>
void VisingoApp::quitBrowser()
{
    clean();
    int tabCount = 0;
    for (int i = 0; i < m_mainWindows.count(); ++i) {
        tabCount += m_mainWindows.at(i)->tabWidget()->count();
    }

    if (tabCount > 1) {
        int ret = QMessageBox::warning(mainWindow(), QString(),
                    QString("There are %1 windows and %2 tabs open\n"
                            "Do you want to quit anyway?").arg(m_mainWindows.count(), tabCount),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No);

        if (ret == QMessageBox::No)
            return;
    }

    exit(0);
}
#endif

/*!
    Any actions that can be delayed until the window is visible
 */
void VisingoApp::postLaunch()
{
    // TODO: remove icon and offline storages
    QString directory = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    if (directory.isEmpty())
        directory = QDir::homePath() + QLatin1String("/.") + QCoreApplication::applicationName();
#if defined(QWEBENGINESETTINGS_PATHS)
    QWebEngineSettings::setIconDatabasePath(directory);
    QWebEngineSettings::setOfflineStoragePath(directory);
#endif
    // END TODO

    // TODO set to visingo icon
    setWindowIcon(QIcon(QLatin1String(":browser.svg")));

    loadSettings();

    // newMainWindow() needs to be called in main() for this to happen
    if (m_mainWindows.count() > 0) {
        mainWindow()->slotHome();
    }
}

void VisingoApp::loadSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));

    QWebEngineSettings *defaultSettings = QWebEngineSettings::globalSettings();
    QWebEngineProfile *defaultProfile = QWebEngineProfile::defaultProfile();

    QString standardFontFamily = defaultSettings->fontFamily(QWebEngineSettings::StandardFont);
    int standardFontSize = defaultSettings->fontSize(QWebEngineSettings::DefaultFontSize);
    QFont standardFont = QFont(standardFontFamily, standardFontSize);
    standardFont = qvariant_cast<QFont>(settings.value(QLatin1String("standardFont"), standardFont));
    defaultSettings->setFontFamily(QWebEngineSettings::StandardFont, standardFont.family());
    defaultSettings->setFontSize(QWebEngineSettings::DefaultFontSize, standardFont.pointSize());

    QString fixedFontFamily = defaultSettings->fontFamily(QWebEngineSettings::FixedFont);
    int fixedFontSize = defaultSettings->fontSize(QWebEngineSettings::DefaultFixedFontSize);
    QFont fixedFont = QFont(fixedFontFamily, fixedFontSize);
    fixedFont = qvariant_cast<QFont>(settings.value(QLatin1String("fixedFont"), fixedFont));
    defaultSettings->setFontFamily(QWebEngineSettings::FixedFont, fixedFont.family());
    defaultSettings->setFontSize(QWebEngineSettings::DefaultFixedFontSize, fixedFont.pointSize());

    // TODO remove unnecessary
    //defaultSettings->setAttribute(QWebEngineSettings::JavascriptEnabled, settings.value(QLatin1String("enableJavascript"), true).toBool());
    //defaultSettings->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, settings.value(QLatin1String("enableScrollAnimator"), true).toBool());
    defaultSettings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    defaultSettings->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);

#if defined(QTWEBENGINE_PLUGINS)
    defaultSettings->setAttribute(QWebEngineSettings::PluginsEnabled, settings.value(QLatin1String("enablePlugins"), true).toBool());
#endif


    // TODO remove unnecessary
    //defaultProfile->setHttpUserAgent(settings.value(QLatin1String("httpUserAgent")).toString());
    defaultProfile->setHttpUserAgent(QString("%1 %2").arg(QCoreApplication::applicationName() + QCoreApplication::applicationVersion()));
    settings.endGroup();
}

QList<BrowserMainWindow*> VisingoApp::mainWindows()
{
    clean();
    QList<BrowserMainWindow*> list;
    for (int i = 0; i < m_mainWindows.count(); ++i)
        list.append(m_mainWindows.at(i));
    return list;
}

void VisingoApp::clean()
{
    // cleanup any deleted main windows
    for (int i = m_mainWindows.count() - 1; i >= 0; --i)
        if (m_mainWindows.at(i).isNull())
            m_mainWindows.removeAt(i);
}

bool VisingoApp::isTheOnlyBrowser() const
{
    return (m_localServer != 0);
}

// TODO: remove? needed?
#if defined(Q_OS_OSX)
bool VisingoApp::event(QEvent* event)
{
    switch (event->type()) {
    case QEvent::ApplicationActivate: {
        clean();
        if (!m_mainWindows.isEmpty()) {
            BrowserMainWindow *mw = mainWindow();
            if (mw && !mw->isMinimized()) {
                mainWindow()->show();
            }
            return true;
        }
    }
    case QEvent::FileOpen:
        if (!m_mainWindows.isEmpty()) {
            mainWindow()->loadPage(static_cast<QFileOpenEvent *>(event)->file());
            return true;
        }
    default:
        break;
    }
    return QApplication::event(event);
}
#endif

void VisingoApp::openUrl(const QUrl &url)
{
    mainWindow()->loadPage(url.toString());
}

// TODO: add slotHome() to send to main page? and remove from elsewhere if needed
BrowserMainWindow *VisingoApp::newMainWindow()
{
    BrowserMainWindow *browser = new BrowserMainWindow();
    m_mainWindows.prepend(browser);
    browser->show();
    return browser;
}

BrowserMainWindow *VisingoApp::mainWindow()
{
    clean();
    if (m_mainWindows.isEmpty())
        newMainWindow();
    return m_mainWindows[0];
}

void VisingoApp::newLocalSocketConnection()
{
    QLocalSocket *socket = m_localServer->nextPendingConnection();
    if (!socket)
        return;
    socket->waitForReadyRead(1000);
    QTextStream stream(socket);
    QString url;
    stream >> url;
    if (!url.isEmpty()) {
        QSettings settings;
        settings.beginGroup(QLatin1String("general"));
        int openLinksIn = settings.value(QLatin1String("openLinksIn"), 0).toInt();
        settings.endGroup();
        if (openLinksIn == 1)
            newMainWindow();
        else
            mainWindow()->tabWidget()->newTab();
        openUrl(url);
    }
    delete socket;
    mainWindow()->raise();
    mainWindow()->activateWindow();
}

// TODO: set icon yourself, disregard icondatabase
QIcon VisingoApp::icon(const QUrl &url) const
{
#if defined(QTWEBENGINE_ICONDATABASE)
    QIcon icon = QWebEngineSettings::iconForUrl(url);
    if (!icon.isNull())
        return icon.pixmap(16, 16);
#else
    Q_UNUSED(url);
#endif
    return defaultIcon();
}

QIcon VisingoApp::defaultIcon() const
{
    if (m_defaultIcon.isNull())
        m_defaultIcon = QIcon(QLatin1String(":defaulticon.png"));
    return m_defaultIcon;
}