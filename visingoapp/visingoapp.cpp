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

#include "bookmarks.h"
#include "browsermainwindow.h"
#include "cookiejar.h"
#include "downloadmanager.h"
#include "history.h"
#include "networkaccessmanager.h"
#include "tabwidget.h"
#include "webview.h"

#include <QtCore/QBuffer>
#include <QtCore/QDir>
#include <QtCore/QLibraryInfo>
#include <QtCore/QSettings>
#include <QtCore/QTextStream>
#include <QtCore/QTranslator>

#include <QtGui/QDesktopServices>
#include <QtGui/QFileOpenEvent>
#include <QtWidgets/QMessageBox>

#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QSslSocket>

#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

#include <QtCore/QDebug>

DownloadManager *VisingoApp::s_downloadManager = 0;
HistoryManager *VisingoApp::s_historyManager = 0;
QNetworkAccessManager *VisingoApp::s_networkAccessManager = 0;
BookmarksManager *VisingoApp::s_bookmarksManager = 0;

static void setUserStyleSheet(QWebEngineProfile *profile, const QString &styleSheet, BrowserMainWindow *mainWindow = 0)
{
    Q_ASSERT(profile);
    QString scriptName(QStringLiteral("userStyleSheet"));
    QWebEngineScript script;
    QList<QWebEngineScript> styleSheets = profile->scripts()->findScripts(scriptName);
    if (!styleSheets.isEmpty())
        script = styleSheets.first();
    Q_FOREACH (const QWebEngineScript &s, styleSheets)
        profile->scripts()->remove(s);

    if (script.isNull()) {
        script.setName(scriptName);
        script.setInjectionPoint(QWebEngineScript::DocumentReady);
        script.setRunsOnSubFrames(true);
        script.setWorldId(QWebEngineScript::ApplicationWorld);
    }
    QString source = QString::fromLatin1("(function() {"\
                                         "var css = document.getElementById(\"_qt_testBrowser_userStyleSheet\");"\
                                         "if (css == undefined) {"\
                                         "    css = document.createElement(\"style\");"\
                                         "    css.type = \"text/css\";"\
                                         "    css.id = \"_qt_testBrowser_userStyleSheet\";"\
                                         "    document.head.appendChild(css);"\
                                         "}"\
                                         "css.innerText = \"%1\";"\
                                         "})()").arg(styleSheet);
    script.setSourceCode(source);
    profile->scripts()->insert(script);
    // run the script on the already loaded views
    // this has to be deferred as it could mess with the storage initialization on startup
    if (mainWindow)
        QMetaObject::invokeMethod(mainWindow, "runScriptOnOpenViews", Qt::QueuedConnection, Q_ARG(QString, source));
}

VisingoApp::VisingoApp(int &argc, char **argv, QString home)
    : QApplication(argc, argv)
    , m_localServer(0)
    , m_privateProfile(0)
    , m_privateBrowsing(false)
    , m_home(home)
{
    QCoreApplication::setOrganizationName(QLatin1String("Aalto University Department of Computer Science, Computational Logic Group"));
    QCoreApplication::setApplicationName(QLatin1String("visingo"));
    QCoreApplication::setApplicationVersion(QLatin1String("0.1"));
    QString serverName = QCoreApplication::applicationName()
        + QString::fromLatin1(QT_VERSION_STR).remove('.') + QLatin1String("webengine");

    // IPC, return if server exists
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

    // IPC, start server
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

    // TODO modify this
    // save argc argv for clingo
    m_saved_argc = argc;
    m_saved_argv = argv;



// TODO: remove if unnecessary
#ifndef QT_NO_OPENSSL
    if (!QSslSocket::supportsSsl()) {
    QMessageBox::information(0, "Visingo",
                 "This system does not support OpenSSL. SSL websites will not be available.");
    }
#endif


    QDesktopServices::setUrlHandler(QLatin1String("http"), this, "openUrl");
    QString localSysName = QLocale::system().name();

    installTranslator(QLatin1String("qt_") + localSysName);

    QSettings settings;
    settings.beginGroup(QLatin1String("sessions"));
    m_lastSession = settings.value(QLatin1String("lastSession")).toByteArray();
    settings.endGroup();

#if defined(Q_OS_OSX)
    connect(this, SIGNAL(lastWindowClosed()),
            this, SLOT(lastWindowClosed()));
#endif

    QTimer::singleShot(0, this, SLOT(postLaunch()));
}

VisingoApp::~VisingoApp()
{
    delete s_downloadManager;
    for (int i = 0; i < m_mainWindows.size(); ++i) {
        BrowserMainWindow *window = m_mainWindows.at(i);
        delete window;
    }
    delete s_networkAccessManager;
    delete s_bookmarksManager;
}

#if defined(Q_OS_OSX)
void VisingoApp::lastWindowClosed()
{
    clean();
    BrowserMainWindow *mw = new BrowserMainWindow(m_saved_argc, m_saved_argv); // TODO: remove when argc argv passing fixed
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
                           tr("There are %1 windows and %2 tabs open\n"
                              "Do you want to quit anyway?").arg(m_mainWindows.count()).arg(tabCount),
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
    QString directory = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    if (directory.isEmpty())
        directory = QDir::homePath() + QLatin1String("/.") + QCoreApplication::applicationName();
#if defined(QWEBENGINESETTINGS_PATHS)
    QWebEngineSettings::setIconDatabasePath(directory);
    QWebEngineSettings::setOfflineStoragePath(directory);
#endif

    setWindowIcon(QIcon(QLatin1String(":browser.svg")));

    loadSettings();

    // newMainWindow() needs to be called in main() for this to happen
    if (m_mainWindows.count() > 0) {
        QStringList args = QCoreApplication::arguments();
        if (args.count() > 1)
            mainWindow()->loadPage(args.last());
        else
            mainWindow()->slotHome();
    }
    VisingoApp::historyManager();
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

    defaultSettings->setAttribute(QWebEngineSettings::JavascriptEnabled, settings.value(QLatin1String("enableJavascript"), true).toBool());
    defaultSettings->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, settings.value(QLatin1String("enableScrollAnimator"), true).toBool());

#if defined(QTWEBENGINE_PLUGINS)
    defaultSettings->setAttribute(QWebEngineSettings::PluginsEnabled, settings.value(QLatin1String("enablePlugins"), true).toBool());
#endif

    QString css = settings.value(QLatin1String("userStyleSheet")).toString();
    setUserStyleSheet(defaultProfile, css, mainWindow());

    defaultProfile->setHttpUserAgent(settings.value(QLatin1String("httpUserAgent")).toString());
    settings.endGroup();
    settings.beginGroup(QLatin1String("cookies"));

    QWebEngineProfile::PersistentCookiesPolicy persistentCookiesPolicy = QWebEngineProfile::PersistentCookiesPolicy(settings.value(QLatin1String("persistentCookiesPolicy")).toInt());
    defaultProfile->setPersistentCookiesPolicy(persistentCookiesPolicy);
    QString pdataPath = settings.value(QLatin1String("persistentDataPath")).toString();
    defaultProfile->setPersistentStoragePath(pdataPath);

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
    // cleanup any deleted main windows first
    for (int i = m_mainWindows.count() - 1; i >= 0; --i)
        if (m_mainWindows.at(i).isNull())
            m_mainWindows.removeAt(i);
}

void VisingoApp::saveSession()
{
    if (m_privateBrowsing)
        return;

    clean();

    QSettings settings;
    settings.beginGroup(QLatin1String("sessions"));

    QByteArray data;
    QBuffer buffer(&data);
    QDataStream stream(&buffer);
    buffer.open(QIODevice::ReadWrite);

    stream << m_mainWindows.count();
    for (int i = 0; i < m_mainWindows.count(); ++i)
        stream << m_mainWindows.at(i)->saveState();
    settings.setValue(QLatin1String("lastSession"), data);
    settings.endGroup();
}

bool VisingoApp::canRestoreSession() const
{
    return !m_lastSession.isEmpty();
}

void VisingoApp::restoreLastSession()
{
    QList<QByteArray> windows;
    QBuffer buffer(&m_lastSession);
    QDataStream stream(&buffer);
    buffer.open(QIODevice::ReadOnly);
    int windowCount;
    stream >> windowCount;
    for (int i = 0; i < windowCount; ++i) {
        QByteArray windowState;
        stream >> windowState;
        windows.append(windowState);
    }
    for (int i = 0; i < windows.count(); ++i) {
        BrowserMainWindow *newWindow = 0;
        if (m_mainWindows.count() == 1
            && mainWindow()->tabWidget()->count() == 1
            && mainWindow()->currentTab()->url() == QUrl()) {
            newWindow = mainWindow();
        } else {
            newWindow = newMainWindow();
        }
        newWindow->restoreState(windows.at(i));
    }
}

bool VisingoApp::isTheOnlyBrowser() const
{
    return (m_localServer != 0);
}

void VisingoApp::installTranslator(const QString &name)
{
    QTranslator *translator = new QTranslator(this);
    translator->load(name, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    QApplication::installTranslator(translator);
}

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

BrowserMainWindow *VisingoApp::newMainWindow()
{
    BrowserMainWindow *browser = new BrowserMainWindow(m_saved_argc, m_saved_argv);
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

CookieJar *VisingoApp::cookieJar()
{
#if defined(QWEBENGINEPAGE_SETNETWORKACCESSMANAGER)
    return (CookieJar*)networkAccessManager()->cookieJar();
#else
    return 0;
#endif
}

DownloadManager *VisingoApp::downloadManager()
{
    if (!s_downloadManager) {
        s_downloadManager = new DownloadManager();
    }
    return s_downloadManager;
}

QNetworkAccessManager *VisingoApp::networkAccessManager()
{
#if defined(QWEBENGINEPAGE_SETNETWORKACCESSMANAGER)
    if (!s_networkAccessManager) {
        s_networkAccessManager = new NetworkAccessManager();
        s_networkAccessManager->setCookieJar(new CookieJar);
    }
    return s_networkAccessManager;
#else
    if (!s_networkAccessManager) {
        s_networkAccessManager = new QNetworkAccessManager();
    }
    return s_networkAccessManager;
#endif
}

HistoryManager *VisingoApp::historyManager()
{
    if (!s_historyManager)
        s_historyManager = new HistoryManager();
    return s_historyManager;
}

BookmarksManager *VisingoApp::bookmarksManager()
{
    if (!s_bookmarksManager) {
        s_bookmarksManager = new BookmarksManager;
    }
    return s_bookmarksManager;
}

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

void VisingoApp::setPrivateBrowsing(bool privateBrowsing)
{
    if (m_privateBrowsing == privateBrowsing)
        return;
    m_privateBrowsing = privateBrowsing;
    if (privateBrowsing) {
        if (!m_privateProfile)
            m_privateProfile = new QWebEngineProfile(this);
        Q_FOREACH (BrowserMainWindow* window, mainWindows()) {
            window->tabWidget()->setProfile(m_privateProfile);
        }
    } else {
        Q_FOREACH (BrowserMainWindow* window, mainWindows()) {
            window->tabWidget()->setProfile(QWebEngineProfile::defaultProfile());
            window->m_lastSearch = QString::null;
            window->tabWidget()->clear();
        }
    }
    emit privateBrowsingChanged(privateBrowsing);
}
