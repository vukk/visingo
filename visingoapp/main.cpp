/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
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
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
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

#include "visingoapp.h"

// qtwebserver
#include "tcp/tcpmultithreadedserver.h"
#include "http/httpwebengine.h"
#include "http/httpiodeviceresource.h"

#include <qtwebenginewidgetsglobal.h>

#include <QLoggingCategory>


// webserver resource, TODO MOVE TO SEPARATE FILE
#include <QFile>
#include <QDir>
#include <QMimeDatabase>

using namespace QtWebServer;


// TODO: move to separate file
class DirResource : public Http::Resource {
public:
    DirResource(QDir *webRootDir) : Http::Resource("/"), _webRootDir(webRootDir) {
        qDebug() << "webRoot: " << _webRootDir->path();
    } // root "/" is dummy, we catch all

    ~DirResource() { }

    void deliver(const Http::Request& request, Http::Response& response) {
        //response.setStatusCode(Http::Ok);
        //response.setHeader(Http::ContentType, "text/html");

        QString uri = request.uniqueResourceIdentifier();
        if (uri.isEmpty() || uri == "/") {
            uri = QString("index.html");
        }
        if (uri.startsWith('/')) {
            uri = uri.mid(1);
        }

        qDebug() << "req uri: " << uri;
        //qDebug() << "filePath: " << _webRootDir->filePath(uri);

        if(QFile::exists(_webRootDir->filePath(uri))) {
            if(request.method() == "get") {
                //response.setHeader(Http::ContentType, contentType());

                // TODO CRITICAL: check that file is somewhere inside webRoot
                QFile file(_webRootDir->filePath(uri));

                file.open(QIODevice::ReadOnly);
                if(file.isOpen()) {
                    QFileInfo fileInfo(file);
                    response.setHeader(Http::ContentType, _mimeDatabase.mimeTypeForFile(fileInfo).name());
                    response.setBody(file.readAll());
                    response.setStatusCode(Http::Ok);
                    file.close();
                } else {
                    response.setHeader(Http::ContentType, "text/html");
                    response.setBody(QString(
                        "<h1>Visingo: Forbidden (could not read resource)</h1> <p>Path was: %1</p>").arg(
                            _webRootDir->absoluteFilePath(uri)).toUtf8());
                    response.setStatusCode(Http::Forbidden);
                }
            }
        }
        else {
            response.setHeader(Http::ContentType, "text/html");
            response.setBody(QString(
                "<h1>Visingo: Resource not found.</h1> <p>Path was: %1</p>").arg(
                    _webRootDir->absoluteFilePath(uri)).toUtf8());
            response.setStatusCode(Http::NotFound);
        }
    }

    // TODO: should make a directory serving resource class for QtWebServer
    bool match(QString uniqueIdentifier) {
        // catch all
        Q_UNUSED( uniqueIdentifier )
        return true;
    }
private:
    QMimeDatabase _mimeDatabase;
    QDir *_webRootDir;
};

// TODO CRITICAL: MAKE SURE WE DONT SERVE OUTSIDE OF WEBROOT DIR!

int main(int argc, char **argv)
{
    int port = 7358;
    //Q_INIT_RESOURCE(data);
    VisingoApp visingoapp(argc, argv, QString("http://localhost:%1").arg(port));
    // TODO Remove check below, maybe. Check first if something breaks.
    if (!visingoapp.isTheOnlyBrowser())
        return 0;

    // TODO Ignore SSL errors only on release version (Debian issue)
    //qputenv("QT_LOGGING_RULES", "qt.network.ssl.warning=false");
    QLoggingCategory::setFilterRules("qt.network.ssl.warning=false");

    // add webserver
    //QCoreApplication a(argc, argv);

    QtWebServer::Tcp::MultithreadedServer tcpServer;
    QtWebServer::Http::WebEngine webHandler;

    /*
    Http::IODeviceResource *rsc = new Http::IODeviceResource(
                "/",
                new QFile(QString("%1/%2").arg(visingoapp.applicationDirPath(), "../ui-assets/index.html")));

    rsc->setContentType("text/html");
    webHandler.addResource(rsc);
    */

    webHandler.addResource(
        new DirResource(new QDir(
            QString("%1/%2").arg(visingoapp.applicationDirPath(), "../ui-assets"))));

    /*
    w.addResource(new QtWebServer::Http::IODeviceResource(
                  "/",
                  new QFile(QString("%1/%2").arg(visingoapp.applicationDirPath(), "../ui-assets/index.html"))));
    */

    tcpServer.setResponder(&webHandler);
    tcpServer.listen(QHostAddress::LocalHost, port);


    // TODO output error if not
    if(!tcpServer.isListening()) {
        qDebug() << "Webserver not listening, exiting...";
        return 1;
    }

    // visingoapp
    visingoapp.newMainWindow();
    return visingoapp.exec();
}
