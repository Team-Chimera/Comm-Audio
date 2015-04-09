=================================================
|	Data Communications (COMP 4985)		|
|	Assisgnment #4 - Comm Audio		|
|		Rhea Lauzon			|
|		Julian Brandrick		|
|		Jeff Bayntun			|
|		Michael Chimick			|
=================================================


~~~~~~~~~~~~~~~~
    DOCUMENTS:
~~~~~~~~~~~~~~~~
- design document
- testing document
- user guide


~~~~~~~~~~~~~~~~~~~
    EXECUTABLES:
~~~~~~~~~~~~~~~~~~~

client.exe (REQUIRES QT)

server.exe located in \Executables\Server\SERVER-EXECUTABLE
due to the need for .dll's for VLC



~~~~~~~~~~~~~~~~~~~
    SOURCECODE:
~~~~~~~~~~~~~~~~~~~

=================
CLIENT
=================
client.h
controlchannel.h
controlMessage.h
dialog.h
downloadSong.h
mainwindow.h
microphone.h
multicast.h
music.h
player.h
socketinfo.h
tcpdownload.h
unicastdialog.h
unicastSong.h
voiceDialog.h


controlchannel.cpp
dialog.cpp
downloadSong.cpp
main.cpp
mainwindow.cpp
microphone.cpp
multicast.cpp
tcpdownload.cpp
unicastdialog.cpp
unicastSong.cpp
voiceDialog.cpp


===================
SERVER
===================
helper.h
multicast.h
music.h
network.h
newsession.h
stdafx.h
streamingserver.h
targver.h
unicast.h

helper.cpp
unicast.cpp
streamingServer.cpp
session.cpp
stdafx.cpp
newsession.cpp
network.cpp
multicast.cpp

