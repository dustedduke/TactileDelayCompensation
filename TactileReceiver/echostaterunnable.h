#ifndef ECHOSTATERUNNABLE_H
#define ECHOSTATERUNNABLE_H

#include <QObject>
#include "echostatenetwork.h"
#include <QThread>
#include <QRunnable>
#include <vector>
#include <sys/types.h>
#include <sys/time.h>

class EchoStateRunnable : public QObject, public QRunnable
{
    Q_OBJECT
public:
    EchoStateRunnable(QString dataForPrediction);
    void run();

signals:
    void dataPredicted(QString data);
};

#endif // ECHOSTATERUNNABLE_H
