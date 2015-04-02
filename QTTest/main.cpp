#include <QApplication>
#include <QQmlApplicationEngine>

#include "microphone.h"
#include "player.h"

void runMicrophone();
void runPlayer();

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    Microphone mic;
    Player play;
    
    return app.exec();
}
