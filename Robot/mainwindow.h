#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "networkthread.h"

#include <QObject>
#include <vector>

#include <QCoreApplication>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QDebug>
#include <QTime>
#include <QMap>
#include <QMainWindow>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "qcustomplot.h"
#include "time.h"

#include <QSemaphore>
#include <thread>

#include <QTimer>

using namespace std::chrono;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void createServer();
    void setupPlot();
    void plotCombined(long maxDelayTime, bool &transmissionStarted, QMap<QString, double> &bufferAxis1Pointer, QMap<QString, double> &bufferAxis2Pointer, QMap<QString, double> &bufferAxis3Pointer, high_resolution_clock::time_point t1, long long &currentMessageTimeThread1,
                      long long &currentMessageTimeThread2, long long &currentMessageTimeThread3,
                      float &currentMessageValueThread1, float &currentMessageValueThread2, float &currentMessageValueThread3);
    void startPlotCombined();
    void connectToUnity();
    void sendDataToUnity(QString message);


    ~MainWindow();

public slots:
    void logMessage(QString message, long long delay, long long firstTimeStampFromMessage);
    void logPredictedMessage(QString message, long long delay, long long firstTimeStampFromMessage);
    void plotMessage(QString message, long long delay, long long firstTimeStampFromMessage);
    void plotPredictedMessage(QString message, long long delay, long long firstTimeStampFromMessage);
    void on_addDataPoint(bool predicted, long long x, float y1, float y2, float y3);

    void plotCombined2();


signals:
    void addDataPoint(bool predicted, long long x, float y1, float y2, float y3);

private:
    double key;
    Ui::MainWindow *ui;

//    double currentMessageTime;
//    double currentMessageValue;

//    double lastMessageTime;
//    double lastMessageValue;

};

#endif // MAINWINDOW_H
