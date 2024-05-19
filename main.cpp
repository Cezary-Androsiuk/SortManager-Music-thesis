#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <cpp/Backend/Backend.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    Backend* backend = new Backend(&engine);
    engine.rootContext()->setContextProperty("Backend", backend);

    const QUrl url(u"qrc:/SortManager-Music/qml/Main.qml"_qs);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
