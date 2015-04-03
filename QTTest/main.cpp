#include <QApplication>
#include <QQmlApplicationEngine>

#include "microphone.h"
#include "player.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    Microphone mic("127.0.0.1");
    Player play;
    
    return app.exec();
}
