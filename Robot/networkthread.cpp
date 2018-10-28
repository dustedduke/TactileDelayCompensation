#include "networkthread.h"

#include "networkthread.h"
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
#include <netinet/tcp.h>
#include <chrono>
#include <ctime>

#include <fcntl.h>

//int listenfd = 0, connfd = 0;
//char sendBuff[3500];
//char recvBuff[3500];
//struct msghdr recvHeader;
//struct cmsghdr *cmhdr;

//QString recvString;
//QString predictionBlock;
//QStringList transitionList;

//struct sockaddr_in serv_addr;
//struct sockaddr_in cli_addr;

//std::chrono::time_point<std::chrono::system_clock> chronoStamp1, chronoStamp2;

using namespace std::chrono;

long long firstTimeStamp;
bool timeStampNotUsed;

NetworkThread::NetworkThread()
{

}

void NetworkThread::createConnection(struct sockaddr_in serv_addr, bool realValue)
{
    this->server = true;
    activeConnection = -1;
    this->real = realValue;

    //chronoStamp1 = std::chrono::system_clock::now();
    //chronoStamp2 = std::chrono::system_clock::now();

    //listenfd = socket(AF_INET, SOCK_STREAM, 0); // SOCK_DGRAM для UDP? SOCK_RAW для сырых
    this->listenfd = socket(AF_INET, SOCK_STREAM, 0);

    if(this->listenfd < 0) {
        perror("socket");
        exit(1);
    }

   // int status = fcntl(listenfd, F_SETFL, fcntl(listenfd, F_GETFL, 0) | O_NONBLOCK);

    if(bind(this->listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        exit(2);
    } // (struct sockaddr*) для преобразования из sockaddr_in, который удобнее

    listen(this->listenfd, 1); // 10 - размер очереди запросов

    qDebug() << "NETWORK THREAD CREATED: " << serv_addr.sin_port << endl;

}

int NetworkThread::connectToServer(sockaddr_in serv_addr, QString protocol)
{
    this->server = false;

    int port;

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock < 0) {
        qDebug() << "SOCKET CREATION FAILED" << endl;
        exit(1);
    }

    int yes = 1;
    int result = setsockopt(sock , IPPROTO_TCP, TCP_NODELAY, (char *) &yes, sizeof(int));

    if(result < 0) {
        qDebug() << "ERROR SETTING SOCKET OPTIONS" << endl;
        exit(1);
    }

    qDebug() << "PRE CONNECTED TO ROBOT" << endl;

    if(::connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        QString err = "Socket connection failed";
        qDebug() << err << endl;
        //throw DefaultException(err.toStdString());
        return sock;
        exit(2);
    } else {
        return sock;
    }
}


//Run() с множественными соединениями

void NetworkThread::run() {

    int bytes_read;

    if(this->server == true) {
        while(1) {
            this->connfd = accept(this->listenfd, NULL, NULL); // Accept используется когда сервер готов принять очередной запрос из очереди (создает НОВЫЙ сокет и сообщает дескриптор)
            activeConnection = this->connfd;
            // listenfd - слушающий сокет. Остается слушать после вызова и принимает другие соединения
            // (struct sockaddr*)NULL в структуру записывается адрес сокета? клиента, который установил соединение с сервером

            if(this->connfd < 0) {
                perror("accept");
                exit(3);
            }


            while(1) {

                //bytes_read = recvmmsg(connfd, &recvHeader, 0); // NOT recvmsg!
                bytes_read = recv(this->connfd, this->recvBuff, 20000, 0);

                long long delay = 0;

                this->recvString = this->recvBuff;

                //qDebug() << this->recvString << "\n";

                auto timeFromMessage = this->recvString.split(" ").last().toLongLong();
                auto currentTime = duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
                currentTime = currentTime % 100000000000;
                currentTime = currentTime / 10000;

                delay = currentTime - timeFromMessage;

                long long firstTimeStamp = 0;

                if(this->real == false) {
                    QString temp = this->recvString.split(":").at(1);
                    //firstTimeStamp = temp.split("|").at(0).toLongLong;
                }


                Q_EMIT dataReceived(this->recvString, delay, firstTimeStamp);

                if(bytes_read <= 0) break;
                    send(this->connfd, this->recvBuff, bytes_read, 0);
                }

                //chronoStamp2 = std::chrono::system_clock::now();

                //std::chrono::duration<double> elapsed_seconds = chronoStamp2 - chronoStamp1;

                //qDebug() << "DATA RECEIVED OLD CHRONO" << QString::number(elapsed_seconds.count()) << "\n";

                //chronoStamp1 = chronoStamp2;

                //unsigned long currentCall = high_resolution_clock::now().time_since_epoch() / chrono::milliseconds(1);

                //qDebug() << "DATA RECEIVED: " << QString::number(currentCall)<< endl;



           }

            close(this->connfd);
    }
}


