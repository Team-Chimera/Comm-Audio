#include "mainwindow.h"
#include "session.h"
#include <QApplication>

bool makeSharedSems();

using std::cout;


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    if(!makeSharedSems())
        return 1;
    w.setupConnections();
    w.show();

    return a.exec();
}

bool makeSharedSems()
{
    if( (userAccessSem = CreateSemaphore(NULL, 1, 1, NULL)) == NULL)
    {
        printf("error creating sessionSem\n");
        return false;
    }

    if( (songAccessSem = CreateSemaphore(NULL, 1, 1, NULL)) == NULL)
    {
        printf("error creating sessionSem\n");
        return false;
    }

    if( (newSongSem = CreateSemaphore(NULL, 0, MAX_SESSIONS, NULL)) == NULL)
    {
        printf("error creating sessionSem\n");
        return false;
    }

    if( (userChangeSem = CreateSemaphore(NULL, 0, MAX_SESSIONS, NULL)) == NULL)
    {
        printf("error creating sessionSem\n");
        return false;
    }
}
