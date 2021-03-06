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

#ifndef VISINGO_VISINGOAPP_H
#define VISINGO_VISINGOAPP_H

#include <QtWidgets/QApplication>

#include <QtCore/QUrl>
#include <QtCore/QPointer>

#include <QtGui/QIcon>

QT_BEGIN_NAMESPACE
class QLocalServer;
class QNetworkAccessManager;
class QWebEngineProfile;
QT_END_NAMESPACE

class BookmarksManager;
class BrowserMainWindow;
class CookieJar;
class DownloadManager;
class HistoryManager;
class VisingoApp : public QApplication
{
    Q_OBJECT

public:
    VisingoApp(int &argc, char **argv, QString home);
    ~VisingoApp();
    static VisingoApp *instance();
    void loadSettings();

    bool isTheOnlyBrowser() const;
    BrowserMainWindow *mainWindow();
    QList<BrowserMainWindow*> mainWindows();
    QIcon icon(const QUrl &url) const;
    QIcon defaultIcon() const;

    void saveSession();
    bool canRestoreSession() const;
    bool privateBrowsing() const { return m_privateBrowsing; }

    static HistoryManager *historyManager();
    static CookieJar *cookieJar();
    static DownloadManager *downloadManager();
    static QNetworkAccessManager *networkAccessManager();
    static BookmarksManager *bookmarksManager();

    // TODO: change, use settings dialog maybe
    int argc() const { return m_saved_argc; }
    char **argv() const { return m_saved_argv; }
    QString home() { return m_home; }

#if defined(Q_OS_OSX)
    bool event(QEvent *event);
#endif

public slots:
    BrowserMainWindow *newMainWindow();
    void restoreLastSession();
#if defined(Q_OS_OSX)
    void lastWindowClosed();
    void quitBrowser();
#endif
    void setPrivateBrowsing(bool);

signals:
    void privateBrowsingChanged(bool);

private slots:
    void postLaunch();
    void openUrl(const QUrl &url);
    void newLocalSocketConnection();

private:
    void clean();
    void installTranslator(const QString &name);

    static HistoryManager *s_historyManager;
    static DownloadManager *s_downloadManager;
    static QNetworkAccessManager *s_networkAccessManager;
    static BookmarksManager *s_bookmarksManager;

    QList<QPointer<BrowserMainWindow> > m_mainWindows;
    QLocalServer *m_localServer;
    QByteArray m_lastSession;
    QWebEngineProfile *m_privateProfile;
    bool m_privateBrowsing;
    mutable QIcon m_defaultIcon;

    int m_saved_argc;
    char **m_saved_argv;
    QString m_home;

    //friend class BrowserMainWindow;
};

#endif // VISINGO_VISINGOAPP_H
