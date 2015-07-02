#include "jsvisingocontrol.h"

JSVisingoControl::JSVisingoControl(BrowserMainWindow *mainWindow, QDir *visualizerRootDir, QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_visualizerRootDir(visualizerRootDir)
    , m_loadedVisualizers(QJsonObject{})
{
    //
}

JSVisingoControl::~JSVisingoControl()
{}

QString JSVisingoControl::loadedVisualizers()
{
    qDebug() << QString{QJsonDocument(m_loadedVisualizers).toJson()};
    return QString{QJsonDocument(m_loadedVisualizers).toJson()};
}

bool JSVisingoControl::loadVisualizer(const QString &visualizerName)
{
    qDebug() << "loading visualizer '" << visualizerName << "'";
    // check if exists
    // load define.json if does
    // return define.json by 'emit loaded' or something...
    QString defineJsonPath {QString("%1/%2").arg(visualizerName, "define.json")};
    qDebug() << "definepath: " << defineJsonPath;
    qDebug() << "full: " << m_visualizerRootDir->filePath(defineJsonPath);
    if(QFile::exists(m_visualizerRootDir->filePath(defineJsonPath))) {
        qDebug() << "visualizer define.json exists";
        QFile file(m_visualizerRootDir->filePath(defineJsonPath));
        file.open(QIODevice::ReadOnly);
        if(file.isOpen()) {
            // TODO: check that define.json is OK, QJsonDocument::isObject() etc.
            QJsonObject visDef = QJsonDocument::fromJson(file.readAll()).object();
            m_loadedVisualizers.insert(visualizerName, visDef);
            emit visualizerLoaded(visualizerName);
            file.close();
            return true;
        }
        qDebug() << "could open define.json file";
    }

    // TODO: remove
    QJsonObject test = QJsonObject{};
    test.insert("testKey", "testValue");
    emit sendTestJson(test);
    emit sendTestJsonStr(QString{QJsonDocument(test).toJson()});

    return false;
}
