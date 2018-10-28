#include "mainwindow.h"
#include "ui_mainwindow.h"

NetworkThread* netThread;
NetworkThread* netThread2;

// --------------------Parameters-----------------------
const int port = 7009; // Real data port
const int port2 = 7010; // Predicted data port

std::string server_address = "192.168.1.103";

const int axis1 = 0;
const int axis2 = 1;
const int axis3 = 2;

const int inputUnitsNumber = 1000;
const int outputUnitsNumber = 100;

const long maxDelayTime = 100; //ms
//------------------------END---------------------------

int unityFd = 0;

bool timeStampUsed;
long long realTimeStamp;
long long lastTimeStamp;

bool starterMessageArrived = false;
long long calls = 0;
long long currentTimeStamp;

QMap<QString, double> bufferAxis1;
QMap<QString, double> bufferAxis2;
QMap<QString, double> bufferAxis3;

long long totalError1 = 0;
long long totalError2 = 0;
long long totalError3 = 0;

int predictionsCounter1 = 0;
int predictionsCounter2 = 0;
int predictionsCounter3 = 0;

double lastRealPoint = 0;
bool hasNextMessage = false;


bool plotCombinedStarted = false;
long long globalMessageCounter = 0;

using namespace std::chrono;

high_resolution_clock::time_point t1;
high_resolution_clock::time_point t2;

long long currentMessageTime1 = 0.0;
float currentMessageValue1 = 0.0;
long long currentMessageTime2 = 0.0;
float currentMessageValue2 = 0.0;
long long currentMessageTime3 = 0.0;
float currentMessageValue3 = 0.0;

long long lastMessageTime = 1.0;
float lastMessageValue = 0.0;

bool transmissionStarted = false;

QTimer *timer;
bool havePredictedMessage = false;



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupPlot();
    createServer();
    connectToUnity();

    t1 = high_resolution_clock::now();
    lastMessageTime = 1;
    startPlotCombined();
    connect(this, &MainWindow::addDataPoint, this, &MainWindow::on_addDataPoint);

}



MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createServer()
{

    struct sockaddr_in serv_addr;
    struct sockaddr_in serv_addr2;

    memset(&serv_addr,0,sizeof(serv_addr));
    memset(&serv_addr2,0,sizeof(serv_addr2));//Заполнение нулями

    serv_addr.sin_family = AF_INET;
    //serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_pton(AF_INET, server_address.c_str(), &(serv_addr.sin_addr.s_addr));
    serv_addr.sin_port = htons(port);

    serv_addr2.sin_family = AF_INET;
    //serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_pton(AF_INET, server_address.c_str(), &(serv_addr2.sin_addr.s_addr));
    serv_addr2.sin_port = htons(port2);

    netThread = new NetworkThread();
    netThread->createConnection(serv_addr, true);
    //QObject::connect(netThread, &NetworkThread::dataReceived, this, &MainWindow::logMessage);
    QObject::connect(netThread, &NetworkThread::dataReceived, this, &MainWindow::plotMessage);
    //QObject::connect(netThread, &NetworkThread::dataReceived, this, &MainWindow::plotCombined);

    netThread2 = new NetworkThread();
    netThread2->createConnection(serv_addr2, false);
    QObject::connect(netThread2, &NetworkThread::dataReceived, this, &MainWindow::logPredictedMessage);
    //QObject::connect(netThread2, &NetworkThread::dataReceived, this, &MainWindow::plotPredictedMessage);




    //QObject::connect(netThread, &NetworkThread::dataReceived, this, &MainWindow::updateTimeLabels);

    netThread->start();
    netThread2->start();
}

void MainWindow::connectToUnity()
{
    int port;
    struct sockaddr_in cli_addr;

    memset(&cli_addr, 0, sizeof(cli_addr));
    port = 13000;  //7010

    cli_addr.sin_family = AF_INET;
    //"192.168.1.103"
    //inet_pton(AF_INET, address, &(cli_addr.sin_addr.s_addr));
    inet_pton(AF_INET, "192.168.1.103", &(cli_addr.sin_addr.s_addr));
    cli_addr.sin_port = htons(port);

    NetworkThread *sockThread = new NetworkThread();

    unityFd = sockThread->connectToServer(cli_addr, "TCP");
    qDebug() << "UNITY FD: " << unityFd << "\n";

}

void MainWindow::sendDataToUnity(QString message)
{
    char msg[message.size()];//[1107];
    char msgbuf[message.size()];//[1107];

    int sockDesc = unityFd;

    //qDebug() << "SENDING MESSAGE: " << message.toStdString().c_str() << "\n";
    std::sprintf(msg, "%s", message.toStdString().c_str());
    //std::sprintf(msg + strlen(msg), "%s", message.toStdString().c_str());

    send(sockDesc, msg, sizeof(msg), 0);
    //recv(sockDesc, msgbuf, sizeof(message), 0);
}

void MainWindow::startPlotCombined()
{
    qDebug() << "STARTING THREAD \n";
    std::thread thr(&MainWindow::plotCombined, this, maxDelayTime, std::ref(transmissionStarted), std::ref(bufferAxis1), std::ref(bufferAxis2), std::ref(bufferAxis3), t1, std::ref(currentMessageTime1),
                    std::ref(currentMessageTime2), std::ref(currentMessageTime3), std::ref(currentMessageValue1), std::ref(currentMessageValue2),
                    std::ref(currentMessageValue3));
    thr.detach();
}

void MainWindow::on_addDataPoint(bool predicted, long long x, float y1, float y2, float y3)
{
    //Отправить данные в unity

    if(predicted == true) {
        ui->CombinedPlot->graph(0)->setPen(QPen(Qt::red));
        ui->CombinedPlot->graph(0)->addData(x/100000, y1);
        ui->CombinedPlot->xAxis->setRange(x/100000 + 2, 8, Qt::AlignRight);
        ui->CombinedPlot->replot();
        ui->CombinedPlot->graph(0)->setPen(QPen(Qt::black));

        QString message = QString::number(y1) + " " + QString::number(y2) + " " + QString::number(y3) + "&";

        sendDataToUnity(message);

    } else {
        ui->CombinedPlot->graph(0)->addData(x/100000, y1);
        ui->CombinedPlot->xAxis->setRange(x/100000 + 2, 8, Qt::AlignRight);
        ui->CombinedPlot->replot();

        //qDebug() << "REAL Y " << y1 << "\n";

        QString message = QString::number(y1) + " " + QString::number(y2) + " " + QString::number(y3) + "&";

        sendDataToUnity(message);
    }
}

//Запускается в другом потоке
void MainWindow::plotCombined(long maxDelayTime, bool &transmissionStartedThread, QMap<QString, double> &bufferAxis1Pointer, QMap<QString, double> &bufferAxis2Pointer, QMap<QString, double> &bufferAxis3Pointer, high_resolution_clock::time_point t1, long long &currentMessageTimeThread1,
                              long long &currentMessageTimeThread2, long long &currentMessageTimeThread3,
                              float &currentMessageValueThread1, float &currentMessageValueThread2, float &currentMessageValueThread3)
{
    qDebug() << "INSIDE THREAD\n";

    long long lastMessageTimeThread = 1;
    int calls = 0;
    long long currentTimeStamp;
    high_resolution_clock::time_point t1Thread; //= t1;
    high_resolution_clock::time_point t2Thread;

    long maxDelayTimeThread = maxDelayTime;
    int iteration = 0;

    while(true) {

        if(transmissionStartedThread == false) {
            t1Thread = high_resolution_clock::now();
        }

        if(transmissionStartedThread == true) {

            t2Thread = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>( t2Thread - t1Thread ).count();
            long durationInLong = duration;

            //qDebug() << "DIFFERENCE: " << durationInLong << endl;

            if(durationInLong > maxDelayTimeThread) {
                if(currentMessageTimeThread1 == lastMessageTimeThread) {
                    //qDebug() << "CURR: " << currentMessageTimeThread1 << " LAST: " << lastMessageTimeThread << "\n";
                    //qDebug() << "DURATION IN LONG: " << durationInLong << " MAX: " << maxDelayTimeThread << endl;
                    //qDebug() << "TAKING DATA FROM PREDICTION BUFFER\n";
                    QMap<QString, double> bufferAxis1Copy = bufferAxis1Pointer;
                    QMap<QString, double> bufferAxis2Copy = bufferAxis2Pointer;
                    QMap<QString, double> bufferAxis3Copy = bufferAxis3Pointer;

                    calls++;
                    currentTimeStamp += calls*maxDelayTimeThread;
                    if(bufferAxis1Copy.contains(QString::number(currentTimeStamp)) == true) {

                        qDebug() << "TOOK DATA FROM PREDICTION BUFFER\n";
                        emit addDataPoint(true, currentTimeStamp, bufferAxis1Copy.value(QString::number(currentTimeStamp)),
                                          bufferAxis2Copy.value(QString::number(currentTimeStamp)), bufferAxis3Copy.value(QString::number(currentTimeStamp)));
                        //Послать сообщние в unity
                        //sendDataToUnity(QString::number(bufferAxis1Copy.value(QString::number(currentTimeStamp))));





                    }
                }
            }

            if(currentMessageTimeThread1 != lastMessageTimeThread) {

                //qDebug() << "REAL\n";
                //qDebug() << "CURR: " << currentMessageTimeThread1 << "LAST: " << lastMessageTimeThread << "\n";

                emit addDataPoint(false, currentMessageTimeThread1, currentMessageValueThread1, currentMessageValueThread2, currentMessageValueThread3);
                //Послать сообщние в unity
                //sendDataToUnity(QString::number(currentMessageValueThread));

                lastMessageTimeThread = currentMessageTimeThread1;
                t1Thread = high_resolution_clock::now();
                calls = 0;

            }
        }
    }

}

void MainWindow::plotCombined2()
{
    if(currentMessageTime1 == lastMessageTime) {
        calls++;
        long long currentTimeStamp;

        QMap<QString, double> bufferAxis1Copy = bufferAxis1;
        QMap<QString, double> bufferAxis2Copy = bufferAxis2;
        QMap<QString, double> bufferAxis3Copy = bufferAxis3;

        emit addDataPoint(true, currentTimeStamp, bufferAxis1Copy.value(QString::number(currentTimeStamp)),
                          bufferAxis2Copy.value(QString::number(currentTimeStamp)), bufferAxis3Copy.value(QString::number(currentTimeStamp)));
        qDebug() << "USED PREDICTED DATA\n";
    }

    if(currentMessageTime1 > lastMessageTime) {
        calls = 0;
        emit addDataPoint(false, currentMessageTime1, currentMessageValue1, currentMessageValue2, currentMessageValue3);

        lastMessageTime = currentMessageTime1;
    }
}

void MainWindow::logMessage(QString message, long long delay, long long firstTimeStampFromMessage)
{
    //qDebug() << message << "\n";
    ui->logBrowserReal->append(message + '\n');
    ui->realLatency1->setText(QString::number(delay));
}

void MainWindow::logPredictedMessage(QString message, long long delay, long long firstTimeStampFromMessage)
{
    //ui->logBrowserPredicted->append(message + '\n');
    if(havePredictedMessage == false) {
        havePredictedMessage = true;
    }
}

void MainWindow::setupPlot()
{
    ui->plot1->addGraph();
    ui->plot2->addGraph();
    ui->plot3->addGraph();
    ui->plot4->addGraph();
    ui->CombinedPlot->addGraph();

    ui->plot1->graph(0)->setPen(QPen(Qt::black));
    ui->plot2->graph(0)->setPen(QPen(Qt::black));
    ui->plot3->graph(0)->setPen(QPen(Qt::black));
    ui->plot4->graph(0)->setPen(QPen(Qt::black));
    ui->CombinedPlot->graph(0)->setPen(QPen(Qt::black));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%m:%s:%z");
    ui->plot1->xAxis->setTicker(timeTicker);
    ui->plot1->axisRect()->setupFullAxesBox();
    ui->plot1->yAxis->setRange(-100, 100);

    //QSharedPointer<QCPAxisTickerTime> timeTicker2(new QCPAxisTickerTime);
    //timeTicker2->setTimeFormat("%m:%s:%z");
    ui->plot2->xAxis->setTicker(timeTicker);
    ui->plot2->axisRect()->setupFullAxesBox();
    ui->plot2->yAxis->setRange(-100, 100);

    QSharedPointer<QCPAxisTickerTime> timeTicker3(new QCPAxisTickerTime);
    timeTicker3->setTimeFormat("%m:%s:%z");
    ui->plot3->xAxis->setTicker(timeTicker3);
    ui->plot3->axisRect()->setupFullAxesBox();
    ui->plot3->yAxis->setRange(-100, 100);

    //QSharedPointer<QCPAxisTickerTime> timeTicker4(new QCPAxisTickerTime);
    //timeTicker4->setTimeFormat("%m:%s:%z");
    ui->plot4->xAxis->setTicker(timeTicker3);
    ui->plot4->axisRect()->setupFullAxesBox();
    ui->plot4->yAxis->setRange(-100, 100);

    ui->CombinedPlot->xAxis->setTicker(timeTicker);
    ui->CombinedPlot->axisRect()->setupFullAxesBox();
    ui->CombinedPlot->yAxis->setRange(-100, 100);



    // make left and bottom axes transfer their ranges to right and top axes:
//    connect(ui->plot1->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot1->xAxis2, SLOT(setRange(QCPRange)));
//    connect(ui->plot1->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot1->yAxis2, SLOT(setRange(QCPRange)));

//    connect(ui->plot2->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot2->xAxis2, SLOT(setRange(QCPRange)));
//    connect(ui->plot2->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot2->yAxis2, SLOT(setRange(QCPRange)));

//    connect(ui->plot3->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot3->xAxis2, SLOT(setRange(QCPRange)));
//    connect(ui->plot3->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot3->yAxis2, SLOT(setRange(QCPRange)));

//    connect(ui->plot4->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot4->xAxis2, SLOT(setRange(QCPRange)));
//    connect(ui->plot4->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot4->yAxis2, SLOT(setRange(QCPRange)));

    connect(ui->CombinedPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->CombinedPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->CombinedPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->CombinedPlot->yAxis2, SLOT(setRange(QCPRange)));



//    connect(ui->plot1->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot2->xAxis, SLOT(setRange(QCPRange)));
//    connect(ui->plot1->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->CombinedPlot->xAxis, SLOT(setRange(QCPRange)));
//    connect(ui->plot3->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot4->xAxis, SLOT(setRange(QCPRange)));

//    connect(ui->plot1, SIGNAL(afterReplot()), ui->plot2, SLOT(replot()));
//    connect(ui->plot1, SIGNAL(afterReplot()), ui->CombinedPlot, SLOT(replot()));
//    connect(ui->plot3, SIGNAL(afterReplot()), ui->plot4, SLOT(replot()));

}

void MainWindow::plotMessage(QString message, long long delay, long long firstTimeStampFromMessage)
{
    QStringList pieces = message.split( " " );
    double position1 = pieces.at(axis1).toDouble();
    double position2 = pieces.at(axis2).toDouble();
    double position3 = pieces.at(axis3).toDouble();


    double timeStamp = pieces.last().toDouble();

    lastRealPoint = timeStamp;

    currentMessageTime1 = timeStamp;
    currentMessageTime2 = timeStamp;
    currentMessageTime3 = timeStamp;
    currentMessageValue1 = position1 * 1.0;
    currentMessageValue2 = position2 * 1.0;
    currentMessageValue3 = position3 * 1.0;

    if(transmissionStarted == false && havePredictedMessage == true) {
        transmissionStarted = true;
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(plotCombined2()));
        timer->start(maxDelayTime);
    }

//    if(plotCombinedStarted == false) {
//        plotCombinedStarted == true;
//        t1 = high_resolution_clock::now();
//        this->lastMessageTime = 1;
//        startPlotCombined();
//        //plotCombined();

//    }

    static QTime time(QTime::currentTime());

    // calculate two new data points:
    //this->key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    this->key = timeStamp;
    //qDebug() << "ФОРМАТ ВРЕМЕНИ: " << this->key << "\n";

    static double lastPointKey = 0;
    if (this->key-lastPointKey > 0.0005) {
      QMetaMethod metaMethod = sender()->metaObject()->method(senderSignalIndex());
//      if(metaMethod.name() == "passPredictToMainPlot") {
//          for(int i = 0; i < 20; i++) {
//              position_d = pieces.at(i);
//              point = position_d.toDouble();
//              qDebug() << point << endl;
//              ui->onlinePlot->graph(1)->addData(time.elapsed()/1000.0, point);
//          }
//      }
      if(metaMethod.name() == "dataReceived") {
        //ui->plot1->graph(0)->addData(this->key/100000, position1);
        //ui->plot3->graph(0)->addData(this->key/100000, position2);




        double search = this->key;  //1000000
        int searchInt = search;
        double searchDouble = searchInt;

        QMap<QString, double> bufferAxis1Copy = bufferAxis1;
        QMap<QString, double> bufferAxis2Copy = bufferAxis2;
        QMap<QString, double> bufferAxis3Copy = bufferAxis3;

        //qDebug() << "BUFFER AXIS: " << bufferAxis1Copy.keys() << "\n";
        //qDebug() << "SEARCH DOUBLE: " << searchDouble << "\n";

        //bufferAxis1.contains(&searchDouble);

        if(bufferAxis1Copy.contains(QString::number(searchDouble)) > 0) {
            //qDebug() << "FOUND PREDICTION\n";

            predictionsCounter1++;

            double error = position1 - bufferAxis1Copy.value(QString::number(this->key));
            totalError1 += abs(error);
            ui->error1->setText(QString::number(abs(error)));
            ui->mse1->setText(QString::number(totalError1/predictionsCounter1));
            //Задержку тоже здесь



        }

        if(bufferAxis2Copy.contains(QString::number(searchInt))) {

            predictionsCounter2++;

            double error = position2 - bufferAxis2Copy.value(QString::number(this->key));
            totalError2 += abs(error);
            ui->error2->setText(QString::number(abs(error)));
            ui->mse2->setText(QString::number(totalError2/predictionsCounter2));
        }

      }
      lastPointKey = this->key;
    }
    // make key axis range scroll with the data (at a constant range size of 8):
    //ui->plot1->xAxis->setRange(this->key/100000 + 2, 8, Qt::AlignRight);
    //ui->plot1->replot();

    //ui->CombinedPlot->xAxis->setRange(this->key/100000 + 2, 8, Qt::AlignRight);
    //ui->CombinedPlot->replot();

    //ui->plot3->xAxis->setRange(this->key/100000 + 2, 8, Qt::AlignRight);
    //ui->plot3->replot();

}

void MainWindow::plotPredictedMessage(QString message, long long delay, long long firstTimeStampFromMessage)
{

    //qDebug() << "MESSAGE: " << message << endl;



    QStringList pieces = message.split( ":" );
    QString axisString = pieces.at(0).split(" ").at(1);
    int axis = axisString.toInt();


    QStringList curvePoints = pieces.at(1).split(" ");

    //qDebug() << curvePoints << "\n";

    double point;
    static QTime time(QTime::currentTime());

/*    double key = time.elapsed()/1000.0; */// time elapsed since start of demo, in seconds
    static double lastPointKey = 0;

    switch (axis) {
    case axis1:
    {

        QStringList lastPoint = curvePoints.last().split("|");
        double timeOfLastPoint = lastPoint.at(0).toDouble();

        //qDebug() << lastRealPoint << " " << timeOfLastPoint << "\n";

        ui->predictionDistance1->setText(QString::number(timeOfLastPoint - lastRealPoint));

        QCPCurve *newCurve2 = new QCPCurve(ui->plot2->xAxis, ui->plot2->yAxis);
        QVector<QCPCurveData> dataCurve2(outputUnitsNumber);

        double lastTimeStamp;

        // calculate two new data points:


//        if (this->key-lastPointKey > 0.0005) {
            for(int i = 0; i < outputUnitsNumber; i++) {

                QStringList currentPoint = curvePoints.at(i).split("|");

                //qDebug() << i << ": " << currentPoint << endl;

                double timeStamp = currentPoint.at(0).toDouble(); //Из Long long в double
                point = currentPoint.at(1).toDouble();

                //Будет работать если исправить частоту
                bufferAxis1.insert(QString::number(timeStamp), point);
                for(int j = 0; j < 9; j++) {
                    bufferAxis1.insert(QString::number(timeStamp+(j+1)*100), point); //+1мс
                }

                dataCurve2[i] = QCPCurveData(i, this->key/100000+0.01*i, point);


                //qDebug() << point << "\n";


                if(i == 99) {
                    lastTimeStamp = timeStamp;
                }

                //qDebug() << time.elapsed()/1000.0;
            }
          lastPointKey = this->key;
//        }

        newCurve2->data()->set(dataCurve2, true);
        newCurve2->setPen(QPen(Qt::red));

        //ui->plot2->xAxis->setRange(this->key, 8, Qt::AlignRight);
        ui->plot2->replot();

        break;
    }
    case axis2:
    {
        QStringList lastPoint = curvePoints.last().split("|");
        double timeOfLastPoint = lastPoint.at(0).toDouble();
        ui->predictionDistance2->setText(QString::number(timeOfLastPoint - lastRealPoint));


        QCPCurve *newCurve4 = new QCPCurve(ui->plot4->xAxis, ui->plot4->yAxis);
        QVector<QCPCurveData> dataCurve4(outputUnitsNumber);

        double lastTimeStamp;

        // calculate two new data points:
        if (this->key-lastPointKey > 0.0005) {
            for(int i = 0; i < outputUnitsNumber; i++) {
                QStringList currentPoint = curvePoints.at(i).split("|");
                double timeStamp = currentPoint.at(0).toDouble(); //Из Long long в double
                point = currentPoint.at(1).toDouble();

                bufferAxis2.insert(QString::number(timeStamp), point);
                for(int j = 0; j < 9; j++) {
                    bufferAxis2.insert(QString::number(timeStamp+(j+1)*100), point); //+1мс
                }

                dataCurve4[i] = QCPCurveData(i, this->key/100000+0.01*i, point);

                if(i == 99) {
                    lastTimeStamp = timeStamp;
                }

            }
          lastPointKey = this->key;
        }

        newCurve4->data()->set(dataCurve4, true);
        newCurve4->setPen(QPen(Qt::red));

        //ui->plot4->xAxis->setRange(this->key, 8, Qt::AlignRight);
        ui->plot4->replot();

        break;
    }
    case axis3:
    {
        QStringList lastPoint = curvePoints.last().split("|");
        double timeOfLastPoint = lastPoint.at(0).toDouble();
        ui->predictionDistance2->setText(QString::number(timeOfLastPoint - lastRealPoint));

        double lastTimeStamp;

            for(int i = 0; i < outputUnitsNumber; i++) {
                QStringList currentPoint = curvePoints.at(i).split("|");
                double timeStamp = currentPoint.at(0).toDouble(); //Из Long long в double
                point = currentPoint.at(1).toDouble();

                bufferAxis3.insert(QString::number(timeStamp), point);
                for(int j = 0; j < 9; j++) {
                    bufferAxis3.insert(QString::number(timeStamp+(j+1)*100), point); //+1мс
                }

                if(i == 99) {
                    lastTimeStamp = timeStamp;
                }

            }

        break;
    }
    default:
        break;
    }





    // make key axis range scroll with the data (at a constant range size of 8):

}
