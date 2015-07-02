#ifndef VISINGOAPP_JSVISINGOCONTROL_H
#define VISINGOAPP_JSVISINGOCONTROL_H

#include "browsermainwindow.h"

// test object, TODO: move to own file, separate header file also
// methods possibly:
// loadVisualizer -> false if not found, define.json if found
// startVisualizer -> open visualizer in new tab
#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
/*!
    An instance of this class gets published over the WebChannel and is then accessible to HTML clients.
    See http://doc.qt.io/qt-5/qtwebchannel-standalone-main-cpp.html
*/
class JSVisingoControl : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString loadedVisualizers READ loadedVisualizers NOTIFY visualizerLoaded)

public:
    explicit JSVisingoControl(BrowserMainWindow *mainWindow, QDir *visualizerRootDir, QObject *parent = 0);
    virtual ~JSVisingoControl();

public:
    /*
      Helper functions
    */
    QString loadedVisualizers();

signals:
    /*!
        This signal is emitted from the C++ side and the text displayed on the HTML client side.
    */
    // emitted when a visualizer is loaded (or updated) in loadedVisualizers
    void visualizerLoaded(const QString &name);
    void sendTestJson(const QJsonObject &jsonObj);
    void sendTestJsonStr(const QString &json);
    //void visualizerLoaded(const QString &name); // emitted when a visualizer is loaded (or updated) in loadedVisualizers
    //void sendTestJson(const QJsonObject &jsonObj);

public slots:
    /*!
        This slot is invoked from the HTML client side and the text displayed on the server side.
    */
    bool loadVisualizer(const QString &visualizerName);

protected slots:
    /*!
        Note that this slot is private and thus not accessible to HTML clients.
    */
    // none

private:
    BrowserMainWindow *m_mainWindow;
    QDir *m_visualizerRootDir;

    // published
    QJsonObject m_loadedVisualizers;
};
// end test object


#endif // VISINGOAPP_JSVISINGOCONTROL_H
