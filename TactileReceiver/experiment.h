#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <QString>
#include <QObject>
#include <QtCharts>
#include <QMap>
#include <QThread>
#include <QThreadPool>
#include <stdio.h>
#include <ctime>
#include <stdexcept>
#include <vector>
#include "echostatenetwork.h"
#include "echostaterunnable.h"



const int REPEAT_EXPERIMENTS = 10;
//const string DSTAT_DATASET_PATH = "Experiments/data/dstat.data";
//const string MANIPULATOR_DATASET_PATH = "Experiments/data/dstat.data";

using std::cout;
using std::endl;
using std::vector;
using std::clock;
using std::clock_t;

class Experiment: public QObject
{
    Q_OBJECT

public:
    Experiment(int networkType, int experimentType, QString dataPath, QMap<QString, QString> params);
    ~Experiment();

    void ExperimentCWDepth(QString datasetPath, int maxDepth, int hiddenUnits, vector<int> clockRate, float learningRate, float momentumRate);
    void ExperimentEchoState(QString dataPath, int samplesNumber, int inputsNumber, int outputsNumber, int resSize, float alpha, float sparsity);
    void ExperimentEchoStateOnline(int samplesNumber, int inputsNumber, int outputsNumber, int resSize, float alpha, float sparsity);


signals:
    //void experimentFinished(vector<vector<float>> result);
    void experimentFinished(QString result);
    void onlinePredictionFinished(QString result);
    void passPredictToMainPlot(QString data);

    //TODO: расписать multiple experiment
    //void multipleExperimentFinished(vector<vector<float>> result);

public slots:
    void predictOnline(QString data);
    void printPredictedData(QString data);

    void predictOnlineCW(QString data);
    void printPredictedDataCW(QString data);

};



#endif // EXPERIMENT_H
