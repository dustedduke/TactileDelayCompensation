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

#include <chrono>
#include <ctime>

#include <fcntl.h>

int listenfd = 0, connfd = 0;
char sendBuff[40];
char recvBuff[40];
struct msghdr recvHeader;
struct cmsghdr *cmhdr;

QString recvString;
QString predictionBlock;
QStringList transitionList;

struct sockaddr_in serv_addr;
struct sockaddr_in cli_addr;

//std::chrono::time_point<std::chrono::system_clock> chronoStamp1, chronoStamp2;

using namespace std::chrono;

NetworkThread::NetworkThread()
{

}

void NetworkThread::createConnection(struct sockaddr_in serv_addr)
{
    activeConnection = -1;

    //chronoStamp1 = std::chrono::system_clock::now();
    //chronoStamp2 = std::chrono::system_clock::now();

    //listenfd = socket(AF_INET, SOCK_STREAM, 0); // SOCK_DGRAM для UDP? SOCK_RAW для сырых
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    if(listenfd < 0) {
        perror("socket");
        exit(1);
    }

   // int status = fcntl(listenfd, F_SETFL, fcntl(listenfd, F_GETFL, 0) | O_NONBLOCK);

    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        exit(2);
    } // (struct sockaddr*) для преобразования из sockaddr_in, который удобнее

    listen(listenfd, 1); // 10 - размер очереди запросов
}


//Run() с множественными соединениями

void NetworkThread::run() {

    int bytes_read;
    int modPredictionCounter = 0;
    int previousModPredictionCounter = 0;

    int sample_shift = 10;

    bool passedFirstTw = false;

    while(1) {
        connfd = accept(listenfd, NULL, NULL); // Accept используется когда сервер готов принять очередной запрос из очереди (создает НОВЫЙ сокет и сообщает дескриптор)
        activeConnection = connfd;
        // listenfd - слушающий сокет. Остается слушать после вызова и принимает другие соединения
        // (struct sockaddr*)NULL в структуру записывается адрес сокета? клиента, который установил соединение с сервером

        if(connfd < 0) {
            perror("accept");
            exit(3);
        }


        while(1) {

            //bytes_read = recvmmsg(connfd, &recvHeader, 0); // NOT recvmsg!
            bytes_read = recv(connfd, recvBuff, 1024, 0);

            if(bytes_read != 0) {
                modPredictionCounter++;
            }

            //chronoStamp2 = std::chrono::system_clock::now();

            //std::chrono::duration<double> elapsed_seconds = chronoStamp2 - chronoStamp1;

            //qDebug() << "DATA RECEIVED OLD CHRONO" << QString::number(elapsed_seconds.count()) << "\n";

            //chronoStamp1 = chronoStamp2;

            unsigned long currentCall = high_resolution_clock::now().time_since_epoch() / std::chrono::milliseconds(1);


            //qDebug() << "DATA RECEIVED: " << QString::number(currentCall)<< endl;

            recvString = recvBuff;

            predictionBlock.append(recvString.split(" ").at(0));
            predictionBlock.append(" ");


            //qDebug() << "Prediction block: " << predictionBlock.split(" ") << endl;


            // Ответ  по conndf
            if(bytes_read <= 0) break;
            send(connfd, recvBuff, bytes_read, 0);



            if(modPredictionCounter == 300) {
                Q_EMIT predictionBlockReceived(predictionBlock);

                //predictionBlock.clear() нельзя, т.к. через смещение
                //Удаление первых sample_shift элементов, т.к. больше не нужны
                transitionList = predictionBlock.split(" ");

                for(int i = 0; i < sample_shift; i++) {
                    transitionList.removeFirst();
                }
                predictionBlock = transitionList.join(" ");

                previousModPredictionCounter = modPredictionCounter;
                passedFirstTw = true;
            }

            if((modPredictionCounter - previousModPredictionCounter == sample_shift) && (passedFirstTw == true)) {
                Q_EMIT predictionBlockReceived(predictionBlock);

                transitionList = predictionBlock.split(" ");

                for(int i = 0; i < sample_shift; i++) {
                    transitionList.removeFirst();
                }
                predictionBlock = transitionList.join(" ");

                previousModPredictionCounter = modPredictionCounter;
            }

            Q_EMIT dataReceived(recvString);
        }

        close(connfd);
    }

}



//UDP

//void NetworkThread::createConnection(struct sockaddr_in s_addr)
//{
//    activeConnection = -1;

//    //listenfd = socket(AF_INET, SOCK_STREAM, 0); // SOCK_DGRAM для UDP? SOCK_RAW для сырых
//    listenfd = socket(AF_INET, SOCK_DGRAM, 0);

//    if(listenfd < 0) {
//        perror("socket");
//        exit(1);
//    }



//    serv_addr = s_addr;

//    cli_addr.sin_family = AF_INET;
//    //cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//    inet_pton(AF_INET, "192.168.1.129", &(cli_addr.sin_addr.s_addr));
//    cli_addr.sin_port = htons(5009);




//   // int status = fcntl(listenfd, F_SETFL, fcntl(listenfd, F_GETFL, 0) | O_NONBLOCK);

//    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
//        perror("bind");
//        exit(2);
//    } // (struct sockaddr*) для преобразования из sockaddr_in, который удобнее

//    //listen(listenfd, 1); // 10 - размер очереди запросов
//}

//void NetworkThread::run()
//{

//    //time_t ticks;

//    int bytes_read;
//    socklen_t cli_size = sizeof(cli_addr);

//     while(1) {

//         if ((bytes_read = recvfrom(listenfd, recvBuff, sizeof(recvBuff), 0, (struct sockaddr *) &cli_addr, &cli_size)) == -1)
//         {
//            perror("recvfrom()");
//         }


//         recvString = recvBuff;

//         //qDebug() << recvString;

//         emit dataReceived(recvString);

////         if(bytes_read <= 0) break;
////         send(connfd, recvBuff, bytes_read, 0);
//     }

//     close(connfd);


//}

