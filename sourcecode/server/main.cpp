/*---------------------------------------------------------------------------------------------
-- SOURCE FILE: main.cpp -
--
-- PROGRAM:
--
-- DATE: March 22, 2015
--
-- Interface:
-- int main(int argc, char *argv[])
    bool makeSharedSems();
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Rhea Lauzon, Jeff Bayntun
--
-- PROGRAMMER: Rhea Lauzon, Jeff Bayntun
--
--------------------------------------------------------------------------------------------*/

#include "mainwindow.h"
#include "session.h"
#include <QApplication>

bool makeSharedSems();

using std::cout;
/*******************************************************************
** Function: main()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon, Jeff Bayntun
**
** Programmer: Rhea Lauzon, Jeff Bayntun
**
** Interface:
**			argc: the number of command line arguments
**          argv: array with values for each command line argument
**
**
** Returns:
**			0 on success
**
** Notes:
*******************************************************************/
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
/*******************************************************************
** Function: makeSharedSems()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			bool makeSharedSems()
**
**
** Returns:
**			true on success
**
** Notes:
** create semaphores that will be used by all session threads, including
 * for changes in the list of users or the currenly multicasted song
*******************************************************************/
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

    return true;
}
