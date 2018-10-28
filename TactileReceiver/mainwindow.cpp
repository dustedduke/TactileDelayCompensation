#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSignalSpy>
#include <QTest>

QFile file("out.csv");
QFile file2("out2.csv");
QTextStream stream(&file);
QString openFileName;
int fileParseCounter = 1;
int readLinesCounter = 0;

bool firstMessageArrived = false;

bool hasActiveConnection = false;

Experiment *experiment;

NetworkThread* netThread;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupPlot();
    setupRecordPlot();
    setupNeuralPlot();

    ui->runButton->setEnabled(false);

    //Создание списков
    ui->networkTypeCombo->addItem("Both", 1); //0
    ui->networkTypeCombo->addItem("CW Recurrent Network", 2); //1
    ui->networkTypeCombo->addItem("Echo State Network", 3); //2


    ui->actionTypeCombo->addItem("Train", 1);
    ui->actionTypeCombo->addItem("Test", 2);
    ui->actionTypeCombo->addItem("Online Test", 3);

    ui->recordTypeCombo->addItem("X", 1);
    ui->recordTypeCombo->addItem("XYZ", 2);
    ui->recordTypeCombo->addItem("Acc + Gyro", 3);

    connect(ui->networkTypeCombo, SIGNAL(currentTextChanged(const QString &)), this, SLOT(hideItems(QString)));
    connect(ui->inputsNumberField, SIGNAL(textChanged(const QString &)), this, SLOT(parametersTextChanged(QString)));
    connect(ui->outputsNumberField, SIGNAL(textChanged(const QString &)), this, SLOT(parametersTextChanged(QString)));
    connect(ui->learningRateField, SIGNAL(textChanged(const QString &)), this, SLOT(parametersTextChanged(QString)));
    connect(ui->hiddenUnitsField, SIGNAL(textChanged(const QString &)), this, SLOT(parametersTextChanged(QString)));
    connect(ui->momentumRateField, SIGNAL(textChanged(const QString &)), this, SLOT(parametersTextChanged(QString)));
    connect(ui->depthField, SIGNAL(textChanged(const QString &)), this, SLOT(parametersTextChanged(QString)));

    connect(ui->reservoirSizeField, SIGNAL(textChanged(const QString &)), this, SLOT(parametersTextChanged(QString)));
    connect(ui->spectralRadiusField, SIGNAL(textChanged(const QString &)), this, SLOT(parametersTextChanged(QString)));
    connect(ui->sparsityField, SIGNAL(textChanged(const QString &)), this, SLOT(parametersTextChanged(QString)));
    connect(ui->noiseField, SIGNAL(textChanged(const QString &)), this, SLOT(parametersTextChanged(QString)));


}

MainWindow::~MainWindow()
{
    file.remove();
    delete ui;
}


void MainWindow::logMessage(QString message)
{
    ui->logMessages->append(message + '\n');
}

void MainWindow::hideItems(QString networkType) {

    if(networkType == "CW Recurrent Network") {
        ui->EchoStateParamsWidget->hide();
        ui->CWRecurrentParamsWidget->show();
    } else if(networkType == "Echo State Network") {
        ui->CWRecurrentParamsWidget->hide();
        ui->EchoStateParamsWidget->show();
    } else {
        ui->CWRecurrentParamsWidget->show();
        ui->EchoStateParamsWidget->show();
    }
}

void MainWindow::parametersTextChanged(QString networkType) {

    bool CWParamsAreFalse = ui->inputsNumberField->text().isEmpty() || ui->outputsNumberField->text().isEmpty() || ui->learningRateField->text().isEmpty() || ui->hiddenUnitsField->text().isEmpty() || ui->momentumRateField->text().isEmpty() || ui->depth->text().isEmpty();
    bool EchoParamsAreFalse = ui->inputsNumberField->text().isEmpty() || ui->outputsNumberField->text().isEmpty() || ui->reservoirSizeField->text().isEmpty() || ui->spectralRadiusField->text().isEmpty() || ui->sparsityField->text().isEmpty() || ui->noise->text().isEmpty();

    if(networkType == "CW Recurrent Network") {
        if(CWParamsAreFalse) {
            ui->runButton->setEnabled(false);
        } else {
            ui->runButton->setEnabled(true);
        }
    } else if(networkType == "Echo State Network") {
        if(EchoParamsAreFalse) {
            ui->runButton->setEnabled(false);
        } else {
            ui->runButton->setEnabled(true);
        }
    } else {
        if(CWParamsAreFalse && EchoParamsAreFalse) {
            ui->runButton->setEnabled(false);
        } else {
            ui->runButton->setEnabled(true);
        }
    }
}

void MainWindow::getTimeDifference(QString startTime, long int *result)
{
    long int starttime, endtime;
    struct timeval endtm;
    gettimeofday(&endtm, NULL);

    endtime = endtm.tv_usec;
    starttime = startTime.toInt();

    *result = endtime - starttime;
}


void MainWindow::updateTimeLabels(QString message)
{
    long int result;
    QStringList pieces = message.split( " " );

    //Timestamp часто не успевает обрабатываться, поэтому проверка на количество элементов
    if(pieces.size() == 4) {

        QString timeValue = pieces.at(3);
        getTimeDifference(timeValue, &result);

        if(result <= 0) {
            ui->networkLatency->setText("NaN");
        }

        ui->networkLatency->setText("0." + QString::number(result));

    }

    ui->totalLatency->setText("Disabled");

}

void MainWindow::updatePositionLabels(QString message)
{
    QStringList pieces = message.split( " " );

    ui->positionX->setText(pieces.at(0));
    ui->positionY->setText(pieces.at(1));
    ui->positionZ->setText(pieces.at(2));
}


void MainWindow::setupPlot()
{
    QVector<double> x(101), y(101);
    ui->onlinePlot->addGraph();
    ui->onlinePlot->addGraph();

    ui->onlinePlot->graph(0)->setPen(QPen(Qt::black));
    ui->onlinePlot->graph(1)->setPen(QPen(Qt::red));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%m:%s:%z");
    ui->onlinePlot->xAxis->setTicker(timeTicker);
    ui->onlinePlot->axisRect()->setupFullAxesBox();
    ui->onlinePlot->yAxis->setRange(-100, 100);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->onlinePlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->onlinePlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->onlinePlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->onlinePlot->yAxis2, SLOT(setRange(QCPRange)));


    //Prediction plot
    ui->onlinePlot2->addGraph();
    ui->onlinePlot2->addGraph();

    ui->onlinePlot2->graph(0)->setPen(QPen(Qt::black));
    ui->onlinePlot2->graph(1)->setPen(QPen(Qt::red));

    QSharedPointer<QCPAxisTickerTime> timeTicker2(new QCPAxisTickerTime);
    timeTicker2->setTimeFormat("%m:%s:%z");
    //timeTicker2->setTimeFormat("%z");
    ui->onlinePlot2->xAxis->setTicker(timeTicker2);
    ui->onlinePlot2->axisRect()->setupFullAxesBox();
    ui->onlinePlot2->yAxis->setRange(-100, 100);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->onlinePlot2->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->onlinePlot2->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->onlinePlot2->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->onlinePlot2->yAxis2, SLOT(setRange(QCPRange)));



}

void MainWindow::setupRecordPlot()
{
//    QVector<double> x(101), y(101);
//    ui->dataPlot_2->addGraph();
//    ui->dataPlot_2->addGraph();
//    ui->dataPlot_2->addGraph();

//    ui->dataPlot_2->graph(0)->setPen(QPen(Qt::blue));
//    ui->dataPlot_2->graph(1)->setPen(QPen(Qt::green));
//    ui->dataPlot_2->graph(2)->setPen(QPen(Qt::red));

//    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
//    timeTicker->setTimeFormat("%m:%s:%z");
//    ui->dataPlot_2->xAxis->setTicker(timeTicker);
//    ui->dataPlot_2->axisRect()->setupFullAxesBox();
//    ui->dataPlot_2->yAxis->setRange(-50, 50);

//    // make left and bottom axes transfer their ranges to right and top axes:
//    connect(ui->dataPlot_2->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->dataPlot_2->xAxis2, SLOT(setRange(QCPRange)));
//    connect(ui->dataPlot_2->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->dataPlot_2->yAxis2, SLOT(setRange(QCPRange)));
}

void MainWindow::setupNeuralPlot() {

//    QVector<double> x(101), y(101);
//    ui->dataPlot_3->addGraph();
//    ui->dataPlot_3->graph(0)->setPen(QPen(Qt::red));

//    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
//    timeTicker->setTimeFormat("%s");
//    ui->dataPlot_3->xAxis->setTicker(timeTicker);
//    ui->dataPlot_3->axisRect()->setupFullAxesBox();
//    ui->dataPlot_3->yAxis->setRange(0, 100);
//    ui->dataPlot_3->xAxis->setRange(0, 2000);

//    ui->dataPlot_3->xAxis->setLabel("Computation time (s)");
//    ui->dataPlot_3->yAxis->setLabel("Total error");

//    // make left and bottom axes transfer their ranges to right and top axes:
//    connect(ui->dataPlot_3->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->dataPlot_3->xAxis2, SLOT(setRange(QCPRange)));
//    connect(ui->dataPlot_3->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->dataPlot_3->yAxis2, SLOT(setRange(QCPRange)));


//    // dataPlot_4

//    ui->dataPlot_4->addGraph();
//    ui->dataPlot_4->graph(0)->setPen(QPen(Qt::red));

//    timeTicker->setTimeFormat("%s");
//    ui->dataPlot_4->xAxis->setTicker(timeTicker);
//    ui->dataPlot_4->axisRect()->setupFullAxesBox();
//    ui->dataPlot_4->yAxis->setRange(0, 100);
//    ui->dataPlot_4->xAxis->setRange(0, 2000);

//    ui->dataPlot_4->xAxis->setLabel("Computation time (s)");
//    ui->dataPlot_4->yAxis->setLabel("Total error");

//    // make left and bottom axes transfer their ranges to right and top axes:
//    connect(ui->dataPlot_4->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->dataPlot_4->xAxis2, SLOT(setRange(QCPRange)));
//    connect(ui->dataPlot_4->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->dataPlot_4->yAxis2, SLOT(setRange(QCPRange)));


}

void MainWindow::plotMessage(QString message)
{
    //TODO: + plot prediction

    QStringList pieces = message.split( " " );
    QString position_d;
    double point;
    //double position_d = position.toDouble();

    static QTime time(QTime::currentTime());

    // calculate two new data points:
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    static double lastPointKey = 0;
    if (key-lastPointKey > 0.0005) {
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
        position_d = pieces.at(0);
        point = position_d.toDouble();
        ui->onlinePlot->graph(0)->addData(key, point);
      }
      lastPointKey = key;
    }
    // make key axis range scroll with the data (at a constant range size of 8):
    ui->onlinePlot->xAxis->setRange(key, 8, Qt::AlignRight);
    ui->onlinePlot->replot();
}

void MainWindow::plotMessage2(QString message)
{
    // TODO: совместить сообщения в один буфер, находя среднее

    QStringList pieces = message.split( " " );

    qDebug() << message << endl;

    QString position_d;
    QCPCurve *newCurve = new QCPCurve(ui->onlinePlot2->xAxis, ui->onlinePlot2->yAxis);

    double point;
    //double position_d = position.toDouble();
    static QTime time(QTime::currentTime());

    QVector<QCPCurveData> dataCurve(50);

    // calculate two new data points:
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    static double lastPointKey = 0;
    if (key-lastPointKey > 0.0005) {
        for(int i = 0; i < 50; i++) {
            position_d = pieces.at(i);
            point = position_d.toDouble();
            dataCurve[i] = QCPCurveData(i, key + 0.01*i, point);
            //qDebug() << time.elapsed()/1000.0;
        }
      lastPointKey = key;
    }

    newCurve->data()->set(dataCurve, true);
    newCurve->setPen(QPen(Qt::red));

    // make key axis range scroll with the data (at a constant range size of 8):
    ui->onlinePlot2->xAxis->setRange(key, 8, Qt::AlignRight);
    ui->onlinePlot2->replot();
}

//void MainWindow::plotMessage2(QString message)
//{
//    // TODO: совместить сообщения в один буфер, находя среднее

//    QStringList pieces = message.split( " " );
//    QStringList buffer;
//    int sampleShift = 10;

//    if(firstMessageArrived == false) {
//        buffer.append(pieces);
//    }

//    if(firstMessageArrived == true) {

//        buffer.at(buffer.size()-1);
//        for(int i = buffer.size() - 1 - sampleShift; i < buffer.size(); i++) {
//            buffer.at(i) = QString::number((buffer.at(i).toDouble() + pieces.at(i).toDouble())/2);
//        }

//        for(int i = pieces.size() - sampleShift; i < pieces.size(); i++) {
//            buffer.append(pieces.at(i));
//        }
//    }

//    qDebug() << buffer << endl;

//    QString position_d;
//    QCPCurve *newCurve = new QCPCurve(ui->onlinePlot2->xAxis, ui->onlinePlot2->yAxis);

//    double point;
//    //double position_d = position.toDouble();
//    static QTime time(QTime::currentTime());

//    QVector<QCPCurveData> dataCurve(50);

//    // calculate two new data points:
//    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
//    static double lastPointKey = 0;
//    if (key-lastPointKey > 0.0005) {
//        for(int i = buffer.size()-sampleShift; i < buffer.size(); i++) {
//            position_d = buffer.at(i);
//            point = position_d.toDouble();
//            //dataCurve[i] = QCPCurveData(i, key + 0.01*i,
//            ui->onlinePlot2->graph(0)->addData(key + 0.01*i , point);
//            //qDebug() << time.elapsed()/1000.0;
//        }
//      lastPointKey = key;
//    }

//    newCurve->data()->set(dataCurve, true);
//    newCurve->setPen(QPen(Qt::red));

//    // make key axis range scroll with the data (at a constant range size of 8):
//    ui->onlinePlot2->xAxis->setRange(key, 8, Qt::AlignRight);
//    ui->onlinePlot2->replot();
//}


void MainWindow::plotMessageRecord(QString message) {

//    QStringList pieces = message.split( " " );
//    QString positionX = pieces.at(0);
//    double positionX_d = positionX.toDouble();

//    QString positionY = pieces.at(1);
//    double positionY_d = positionY.toDouble();

//    QString positionZ = pieces.at(2);
//    double positionZ_d = positionZ.toDouble();

//    static QTime time(QTime::currentTime());

//    // calculate two new data points:
//    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
//    static double lastPointKey = 0;
//    if (key-lastPointKey > 0.0005) // at most add point every 2 ms
//    {
//      // add data to lines:
//      ui->dataPlot_2->graph(0)->addData(key, positionX_d);
//      ui->dataPlot_2->graph(1)->addData(key, positionY_d);
//      ui->dataPlot_2->graph(2)->addData(key, positionZ_d);
//      lastPointKey = key;
//    }
//    // make key axis range scroll with the data (at a constant range size of 8):
//    ui->dataPlot_2->xAxis->setRange(key, 8, Qt::AlignRight);
//    ui->dataPlot_2->replot();
}


//void MainWindow::plotExperiment(vector<vector<float>> result)
void MainWindow::plotExperiment(QString resultString) {

    qDebug() << "Started plotting\n";

    QVector<double> x(729), y(729);
    QVector<double> y_target(729);// initialize with entries 0..100

    QStringList dataSplit = resultString.split("\n");

    for (int i=0; i<729; ++i)
    {
      double singlePoint = dataSplit.at(i).split(" ").at(1).toDouble();
      x[i] = i; // x goes from -1 to 1
      //y[i] = result[i][0]; // let's plot a quadratic function
      y[i] = singlePoint;
    }

    qDebug() << "Finished reading inputs\n";

    QStringList targetData = dataSplit.at(729).split(" ");

    for(int i = 0; i<729; ++i) {
        //y_target[i] = result[729][i];
        y_target[i] = targetData.at(i).toDouble();
    }

    // create graph and assign data to it:
    ui->resultPlot->addGraph();
    ui->resultPlot->addGraph();

    ui->resultPlot->graph(0)->setData(x, y);
    ui->resultPlot->graph(1)->setData(x, y_target);

    ui->resultPlot->graph(0)->setPen(QPen(Qt::red));
    ui->resultPlot->graph(1)->setPen(QPen(Qt::blue));

    // give the axes some labels:
    ui->resultPlot->xAxis->setLabel("x");
    ui->resultPlot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    ui->resultPlot->xAxis->setRange(0, 800);
    ui->resultPlot->yAxis->setRange(-120, 120);
    ui->resultPlot->replot();
}

void MainWindow::saveToFile(QString message) {

    readLinesCounter++;
    ui->linesRead->setText(QString::number(readLinesCounter));

    QStringList stringPieces = message.split( " " );
    qDebug() << ui->recordTypeCombo->currentIndex() << endl;
    if(ui->recordTypeCombo->currentIndex() == 0) {
        //X

        stream << stringPieces.at(0) << endl;

    } else if(ui->recordTypeCombo->currentIndex() == 1) {
        //XYZ
        stream << stringPieces.at(0) << ", " << stringPieces.at(1) << ", " << stringPieces.at(2) << endl;
    } else if(ui->recordTypeCombo->currentIndex() == 2) {
        //Acc + Gyro
        //stream << stringPieces.at(0) << ", " << stringPieces.at(1) << ", " << stringPieces.at(2) << endl;
    }
    //qDebug() << stringPieces << "\n";
}


void MainWindow::on_connectButton_clicked()
{
    QTextBrowser logMessages;

    int port;
    struct sockaddr_in serv_addr;

    memset(&serv_addr,0,sizeof(serv_addr)); //Заполнение нулями

    port = ui->port->text().toInt();

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //inet_pton(AF_INET, "192.168.1.103", &(serv_addr.sin_addr.s_addr));
    serv_addr.sin_port = htons(port);

    netThread = new NetworkThread();
    netThread->createConnection(serv_addr);
    QObject::connect(netThread, &NetworkThread::dataReceived, this, &MainWindow::logMessage);
    QObject::connect(netThread, &NetworkThread::dataReceived, this, &MainWindow::plotMessage);
    QObject::connect(netThread, &NetworkThread::dataReceived, this, &MainWindow::updateTimeLabels);

    netThread->start();
    hasActiveConnection = true;
}

void MainWindow::on_disconnectButton_clicked()
{
    hasActiveConnection = false;
}

void MainWindow::on_startButton_clicked()
{
    QObject::connect(netThread, &NetworkThread::dataReceived, this, &MainWindow::plotMessageRecord);
    QObject::connect(netThread, &NetworkThread::dataReceived, this, &MainWindow::updatePositionLabels);

    if(file.open(QIODevice::WriteOnly)) {
        if(QObject::connect(netThread, &NetworkThread::dataReceived, this, &MainWindow::saveToFile)) {
            ui->recordInputSize->setDisabled(true);
            ui->recordOutputSize->setDisabled(true);
            ui->logMessages_2->append("Started recording...\n");
        }

    } else {
        ui->logMessages_2->append("ERROR while creating temp file\n");
    }

}

void MainWindow::on_stopButton_clicked()
{
    file.close();
    QObject::disconnect(netThread, &NetworkThread::dataReceived, this, &MainWindow::saveToFile);

    ui->linesRead->setText("0");

    vector<QString> inputWrite;
    vector<QString> targetWrite;

    QString currentLine;

    int countToRead = ui->recordInputSize->text().toInt();
    int countToShift = ui->recordOutputSize->text().toInt();
    int linesInFile = 0;

    file2.open(QIODevice::WriteOnly);
    fileParseCounter = 1;

    file.open(QIODevice::ReadOnly);
    QTextStream countStream (&file);

    while(!countStream.atEnd()) {
        countStream.readLine();
        linesInFile++;
    }

    file.close();
    file.open(QIODevice::ReadOnly);
    QTextStream textStream (&file);

    qDebug() << "Start lines in file\n";

    //linesInFile = ui->linesRead->text().toInt();
    qDebug() << linesInFile << endl;
    // Целочисленное деление на количество входов + 1 чтобы не начинать с 0
    //linesInFile = (linesInFile/countToRead) * countToRead + 1;
    qDebug() << "PRE LINES IN FILE: " << linesInFile << endl;
    linesInFile = (linesInFile/(countToRead+countToShift)) * (countToRead+countToShift);

    qDebug() << "Normalized number of lines in file: " << linesInFile << endl;


    //TODO: сделать подстройку под количество outputs

//    for(int i = 1; i < linesInFile; i++) {
//        if(i % (countToRead + 1) == 0) {
//            for(int j = 0; j < countToShift; j++) {
//                currentLine = textStream.readLine();
//                qDebug() << currentLine << endl;
//                targetWrite.push_back(currentLine);

//            }
//            // !
//            i += countToShift;
//            linesInFile -= countToShift;
//        } else {
//            currentLine = textStream.readLine();
//            inputWrite.push_back(currentLine);
//        }
//    }

    int i = 1;

    int blockcounter = 1;

    while(i < linesInFile) {
        if(blockcounter % (countToRead+1) == 0) {
            blockcounter = 1;
            for(int j = 0; j < countToShift; j++) {
                currentLine = textStream.readLine();
                qDebug() << currentLine << endl;
                targetWrite.push_back(currentLine);
                i++;
            }
        } else {
            currentLine = textStream.readLine();
            inputWrite.push_back(currentLine);
            blockcounter++;
            i++;
        }
    }

    file2.write("INPUT\n");

    int separatorCounter = 0;

    for(int i = 0; i < inputWrite.size(); i++) {
        separatorCounter++;
        file2.write(inputWrite[i].toUtf8());
        if((i < inputWrite.size()-1) && (separatorCounter % countToRead != 0)) {
            file2.write(", ");
        }
        if(separatorCounter % countToRead == 0) {
            file2.write("\n");
        }
    }

    file2.write("\n\nTARGET\n");

    for(int i = 0; i < targetWrite.size(); i++) {
        file2.write(targetWrite[i].toUtf8());
        if(i < targetWrite.size()-1) {
            file2.write(", ");
        }
    }

    file.close();
    file2.close();

    ui->recordInputSize->setDisabled(false);
    ui->recordOutputSize->setDisabled(false);
    linesInFile = 0;

}

void MainWindow::on_saveButton_clicked()
{
    QFileInfo fileInfo(file2.fileName());
    QString oldFileName(fileInfo.fileName());

    QString newFileName = QFileDialog::getSaveFileName(
                        this,
                        tr("Save Document"),
                        QDir::currentPath(),
                        tr("Documents (*.doc)") );
    if( !newFileName.isNull() )
    {
        QFile::copy(oldFileName, newFileName);
        file.remove();
        file2.remove();
    }
}

void MainWindow::on_runButton_clicked()
{
    int networkType = ui->networkTypeCombo->currentIndex();
    int actionType = ui->actionTypeCombo->currentIndex();


    QMap<QString, QString> params;




    if(!ui->dataPath->text().isEmpty()) {

        params.insert("inputsNumber", ui->inputsNumberField->text());
        params.insert("outputsNumber", ui->outputsNumberField->text());

        params.insert("learningRate", ui->learningRateField->text());
        params.insert("hiddenUnits", ui->hiddenUnitsField->text());
        params.insert("momentumRate", ui->momentumRateField->text());
        params.insert("depth", ui->depthField->text());

        params.insert("samplesNumber", ui->samplesNumberField->text());

        params.insert("reservoirUnits", ui->reservoirSizeField->text());
        params.insert("spectralRadius", ui->spectralRadiusField->text());
        params.insert("sparsity", ui->sparsityField->text());
        params.insert("noise", ui->noiseField->text());
        params.insert("inputShift", ui->inputShiftField->text());
        params.insert("inputScaling", ui->inputScalingField->text());

        if(ui->teacherForcingCheckbox->isChecked()) {
            params.insert("teacherForcing", "1.0");
        } else {
            params.insert("teacherForcing", "0.0");
        }

        qRegisterMetaType<vector<vector<float>>>("vector<vector<float>>");

        params.insert("teacherScaling", ui->teacherScalingField->text());
        params.insert("teacherShift", ui->teacherShiftField->text());

        experiment = new Experiment(networkType, actionType, ui->dataPath->text(), params);
        QObject::connect(experiment, &Experiment::experimentFinished, this, &MainWindow::plotExperiment, Qt::QueuedConnection);

        switch(actionType) {
        case 0:
        {   //Train
            if(networkType == 1) {
                //CWRecurrent
                //this->ExperimentCWDepth(dataPath, depth, hiddenUnits, {1,2,4,8}, learningRate, momentumRate);
                //this->ExperimentCWDepth(dataPath, params.value("depth").toInt(), params.value("hiddenUnits").toInt(), {1,2,4,8}, params.value("learningRate").toFloat(), params.value("momentumRate").toFloat());
            }
            if(networkType == 2) {
                //EchoState
                //this->ExperimentEchoState(dataPath, params.value("samplesNumber").toInt(), params.value("inputsNumber").toInt(), params.value("outputsNumber").toInt(), params.value("reservoirUnits").toInt(), params.value("spectralRadius").toFloat(), params.value("sparsity").toFloat());
            }
            if (networkType == 0) {
                //Both
            }
            break;
        }
        case 1:
        {   //Test
            if(networkType == 1) {
                //CWRecurrent
                //this->ExperimentCWDepth(dataPath, depth, hiddenUnits, {1,2,4,8}, learningRate, momentumRate);

                //experiment->ExperimentCWDepth(ui->dataPath->text(), params.value("depth").toInt(), params.value("hiddenUnits").toInt(), {1,2,4,8}, params.value("learningRate").toFloat(), params.value("momentumRate").toFloat());
            }
            if(networkType == 2) {
                //EchoState
                experiment->ExperimentEchoState(ui->dataPath->text(), params.value("samplesNumber").toInt(), params.value("inputsNumber").toInt(), params.value("outputsNumber").toInt(), params.value("reservoirUnits").toInt(), params.value("spectralRadius").toFloat(), params.value("sparsity").toFloat());

            }
            if (networkType == 0) {
                //Both
            }
            break;
        }
        case 2:
        {   //Online test
            if(networkType == 1) {
                //CWRecurrent
            }
            if(networkType == 2) {
                //EchoState
                if(hasActiveConnection) {
                    experiment->ExperimentEchoStateOnline(params.value("samplesNumber").toInt(), params.value("inputsNumber").toInt(), params.value("outputsNumber").toInt(), params.value("reservoirUnits").toInt(), params.value("spectralRadius").toFloat(), params.value("sparsity").toFloat());
                    //Присоединение сетевого потока к нейросети
                    QObject::connect(netThread, &NetworkThread::predictionBlockReceived, experiment, &Experiment::predictOnline);
                    QObject::connect(experiment, &Experiment::passPredictToMainPlot, this, &MainWindow::plotMessage2);
                } else {
                    qDebug() << "ERROR: there is no active connection\n";
                }
            }
            if (networkType == 0) {
                //Both
            }
            break;
        }
        }

        //bool connected = QObject::connect(experiment, SIGNAL(experimentFinished(QString)), this, SLOT(plotExperiment(QString)));
        //qDebug() << "PLOT CONNECTED: " << connected << endl;
    } else {
        qDebug() << "Can't find data with that path\n";
    }


//    if(openFileName == "") {
//        openFileName = "Experiments/nndata2.csv";
//    }

//    int maxDepth = ui->depthValue->text().toInt();
//    int hiddenUnits = ui->hiddenUnitsValue->text().toInt();
//    vector<int> clockRate;
//    float learningRate = 0.01;
//    float momentumRate = 0.9;

//    ExperimentCWDepth(MANIPULATOR, openFileName, maxDepth, hiddenUnits, {1,2,4,8}, learningRate, momentumRate);
}

void MainWindow::on_chooseButton_clicked()
{
    openFileName = QFileDialog::getOpenFileName(this, tr("Open dataset"), "/home", tr("Text files (*.csv, *.tsv, *.data)"));
    qDebug() << openFileName << endl;

    ui->dataPath->setText(openFileName);
}

void MainWindow::on_chooseDataButton_clicked()
{
    openFileName = QFileDialog::getOpenFileName(this, tr("Open dataset"), "/home", tr("All Files (*)"));
    qDebug() << openFileName << endl;

    ui->dataPath->setText(openFileName);
}
