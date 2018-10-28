#ifndef NETWORKTHREAD_H
#define NETWORKTHREAD_H

#include <QObject>

#include <QObject>
#include <QThread>
#include <sys/types.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <iostream>
#include <QDebug>
#include <sys/time.h>

#include <chrono>
#include <ctime>

#include <fcntl.h>

#include "mainwindow.h"

class NetworkThread : public QThread
{
  Q_OBJECT

private:
    int activeConnection; // Для сообщения emit finished()

    int listenfd = 0;
    int connfd = 0;
    char sendBuff[20000];
    char recvBuff[20000];

    bool server;

    struct msghdr recvHeader;
    struct cmsghdr *cmhdr;

    QString recvString;
    QString predictionBlock;
    QStringList transitionList;

    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;

    bool real;


public:
    NetworkThread();
    void createConnection(struct sockaddr_in serv_addr, bool realValue);
    int connectToServer(sockaddr_in serv_addr, QString protocol);
    void run();


signals:
    void dataReceived(QString data, long long delay, long long firstTimeStampFromMessage);
    void timeReveived(QString time); // Посмотреть, можно ли другой формат
    void finished   ();

    void predictionBlockReceived(QString data);

};

#endif // NETWORKTHREAD_H
