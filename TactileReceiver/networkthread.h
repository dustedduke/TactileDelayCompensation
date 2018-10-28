#ifndef NETWORKTHREAD_H
#define NETWORKTHREAD_H
#include <QObject>
#include <QThread>
#include <sys/types.h>
#include <sys/time.h>

#include "mainwindow.h"

class NetworkThread : public QThread
{
  Q_OBJECT

private:
    char sendBuff[40];
    char recvBuff[40];
    int activeConnection; // Для сообщения emit finished()

public:
    NetworkThread();
    void createConnection(struct sockaddr_in serv_addr);
    void run();


signals:
    void dataReceived(QString data);
    void timeReveived(QString time); // Посмотреть, можно ли другой формат
    void finished   ();

    void predictionBlockReceived(QString data);

};

#endif // NETWORKTHREAD_H
