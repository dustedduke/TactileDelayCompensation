#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include <vector>
#include "networkthread.h"
#include "experiment.h"
#include <QMainWindow>
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

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void setupPlot();
    void setupRecordPlot();
    void setupNeuralPlot();
    void getTimeDifference(QString startTime, long int *result);
    ~MainWindow();

public slots:

    void logMessage(QString message);
    void plotMessage(QString message);
    void plotMessage2(QString message);
    void plotMessageRecord(QString message);
    //void plotExperiment(vector<vector<float>> result);
    void plotExperiment(QString result);

    void updateTimeLabels(QString message);
    void updatePositionLabels(QString message);
    void saveToFile(QString message);

private slots:

    void on_connectButton_clicked();

    void on_disconnectButton_clicked();

    void on_startButton_clicked();

    void on_stopButton_clicked();

    void on_saveButton_clicked();

    void on_runButton_clicked();

    void on_chooseButton_clicked();

    void hideItems(QString networkType);
    void parametersTextChanged(QString networkType);

    void on_chooseDataButton_clicked();

private:
    Ui::MainWindow *ui;
};

//Q_DECLARE_METATYPE(vector<vector<float>>)

#endif // MAINWINDOW_H
