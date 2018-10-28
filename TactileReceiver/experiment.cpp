#include "experiment.h"
#include <QDebug>
#include <chrono>
#include "experiment.h"

using namespace std;
using namespace QtCharts;
using namespace std::chrono;

Experiment::Experiment(int networkType, int experimentType, QString dataPath, QMap<QString, QString> params) {

    qDebug() << "Experiment type: " << experimentType << endl;
    qDebug() << "Network type: " << networkType << endl;

}

Experiment::~Experiment() {

}

void Experiment::ExperimentEchoState(QString dataPath, int samplesNumber, int inputsNumber, int outputsNumber, int resSize, float alpha, float sparsity)
{

    qDebug() << samplesNumber << endl;
    qDebug() << "Inputs number" << inputsNumber << endl;

    float** test_inputs = new float*[samplesNumber];

    for(int i = 0; i < samplesNumber; i++) {
        test_inputs[i] = new float[inputsNumber];
    }

    float* test_target = new float[samplesNumber];

    //QFile loadFile("Experiments/bigtest_input");
    //qDebug() << dataPath << endl;
    QFile loadFile(dataPath);
    QTextStream loadStream(&loadFile);

    if(!loadFile.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", loadFile.errorString());
    }

    qDebug() << "Reached reading input\n";

    if(loadStream.readLine() == "INPUT") {
        for(int i = 0; i < samplesNumber; i++) {
            QStringList inputs = loadStream.readLine().split(" ");
            //qDebug() << inputs << endl;
            for(int j = 0; j < inputsNumber; j++) {
                test_inputs[i][j] = inputs.at(j).toFloat();
            }
        }
    } else {
        qDebug() << "Error loading data file\n";
    }

    qDebug() << "Finished reading input\n";

    loadStream.readLine();

    if(loadStream.readLine() == "TARGET") {
        QStringList targets = loadStream.readLine().split(" ");
        for(int i = 0; i < samplesNumber; i++) {
            test_target[i] = targets.at(i).toFloat();
        }
    } else {
        qDebug() << "Error loading data file\n";
    }

    qDebug() << "Finished reading target\n";

    cout << "Files parsed successfully" << endl;
    cout << "ESN trained Depth Experiment" << endl;
    EchoStateNetwork ESN = EchoStateNetwork();
    //float input[20] = {0.5714907356783296,-0.48998162472798623,-0.6924305173690097,0.045713602418041344,0.7350437229186904,-0.19135190070948238,-0.4328778313273105,0.5264213231418309,0.875106597658545,-0.28930070476677905,-0.5353298358107359,-0.3208464528565942,-0.9956752291885742,0.7171889055816869,0.37753383590774026,-0.7105126027978903,0.48283335049186693,-0.09548162337500976,-0.5640076128428777,0.7197587332975541};

    vector<vector<float>> final_result;
    vector<float> prediction;

    for(int i = 0; i < samplesNumber; i++) {
        prediction = ESN.Predict(test_inputs[i], false);
        final_result.push_back(prediction);
        //qDebug() << final_result[i] << "\n";
    }

    vector<float> trgt(test_target, test_target + samplesNumber);;
    vector<float> cwrnn;

    for(int i = 0; i < samplesNumber; i++) {
        qDebug() << trgt[i] << ' ';
    }

    final_result.push_back(trgt);
    //final_result.push_back(cwrnn);

    qDebug() << "Start emitting\n";

    qRegisterMetaType<vector<vector<float>>>("vector<vector<float>>");

    QString resultString;

    QFile plotFile("plotData.csv");
    QTextStream plotStream(&plotFile);

    plotFile.open(QIODevice::WriteOnly);

    for(int i = 0; i < final_result.size(); i++) {
        for(int j = 0; j < final_result[i].size(); j++) {
            resultString.append(QString::number(final_result[i][j]));
            plotFile.write(QString::number(final_result[i][j]).toUtf8());

            resultString.append(" ");
            plotFile.write(" ");
        }
        resultString.append("\n");
        plotFile.write("\n");
    }

    plotFile.close();

    emit experimentFinished(resultString);
    //emit experimentFinished(final_result);
    //Q_EMIT experimentFinished(final_result);






    qDebug() << "Finished emitting\n";


//    //Загрузка тестовых данных

//    float** test_inputs = new float*[148];

//    for(int i = 0; i < 148; i++) {
//        test_inputs[i] = new float[20];
//    }

//    float* test_target = new float[148];

//    //QFile loadFile("Experiments/bigtest_input");
//    QFile loadFile(dataPath);
//    QTextStream loadStream(&loadFile);

//    if(!loadFile.open(QIODevice::ReadOnly)) {
//        QMessageBox::information(0, "error", loadFile.errorString());
//    }


//    for(int i = 0; i < 148; i++) {
//        QStringList inputs = loadStream.readLine().split(" ");
//        for(int j = 0; j < 20; j++) {
//            test_inputs[i][j] = inputs.at(j).toFloat();
//        }
//    }

//    QFile targetFile("Experiments/bigtest_target");
//    QTextStream targetStream(&targetFile);

//    if(!targetFile.open(QIODevice::ReadOnly)) {
//        QMessageBox::information(0, "error", targetFile.errorString());
//    }

//    QStringList targets = targetStream.readLine().split(" ");
//    for(int i = 0; i < 148; i++) {
//        test_target[i] = targets.at(i).toFloat();
//    }

//    cout << "Files parsed successfully" << endl;
//    cout << "ESN trained Depth Experiment" << endl;
//    EchoStateNetwork ESN = EchoStateNetwork();
//    //float input[20] = {0.5714907356783296,-0.48998162472798623,-0.6924305173690097,0.045713602418041344,0.7350437229186904,-0.19135190070948238,-0.4328778313273105,0.5264213231418309,0.875106597658545,-0.28930070476677905,-0.5353298358107359,-0.3208464528565942,-0.9956752291885742,0.7171889055816869,0.37753383590774026,-0.7105126027978903,0.48283335049186693,-0.09548162337500976,-0.5640076128428777,0.7197587332975541};

//    vector<vector<float>> final_result;
//    vector<float> prediction;

//    for(int i = 0; i < 148; i++) {
//        prediction = ESN.Predict(test_inputs[i], false);
//        final_result.push_back(prediction);
//        //qDebug() << final_result[i] << "\n";
//    }

//    vector<float> trgt(test_target, test_target + 148);
//    vector<float> cwrnn;

//    for(int i = 0; i < 148; i++) {
//        qDebug() << trgt[i] << ' ';
//    }

//    final_result.push_back(trgt);
//    final_result.push_back(cwrnn);

//    emit experimentFinished(final_result);


}


void Experiment::ExperimentEchoStateOnline(int samplesNumber, int inputsNumber, int outputsNumber, int resSize, float alpha, float sparsity)
{
//    qDebug() << samplesNumber << endl;
//    qDebug() << "Inputs number" << inputsNumber << endl;

//    float** test_inputs = new float*[samplesNumber];

//    for(int i = 0; i < samplesNumber; i++) {
//        test_inputs[i] = new float[inputsNumber];
//    }

//    float* test_target = new float[samplesNumber];

//    //QFile loadFile("Experiments/bigtest_input");
//    //qDebug() << dataPath << endl;
//    QFile loadFile(dataPath);
//    QTextStream loadStream(&loadFile);

//    if(!loadFile.open(QIODevice::ReadOnly)) {
//        QMessageBox::information(0, "error", loadFile.errorString());
//    }

//    qDebug() << "Reached reading input\n";

//    if(loadStream.readLine() == "INPUT") {
//        for(int i = 0; i < samplesNumber; i++) {
//            QStringList inputs = loadStream.readLine().split(" ");
//            //qDebug() << inputs << endl;
//            for(int j = 0; j < inputsNumber; j++) {
//                test_inputs[i][j] = inputs.at(j).toFloat();
//            }
//        }
//    } else {
//        qDebug() << "Error loading data file\n";
//    }

//    qDebug() << "Finished reading input\n";

//    loadStream.readLine();

//    if(loadStream.readLine() == "TARGET") {
//        QStringList targets = loadStream.readLine().split(" ");
//        for(int i = 0; i < samplesNumber; i++) {
//            test_target[i] = targets.at(i).toFloat();
//        }
//    } else {
//        qDebug() << "Error loading data file\n";
//    }

//    qDebug() << "Finished reading target\n";

//    cout << "Files parsed successfully" << endl;
//    cout << "ESN trained Depth Experiment" << endl;
//    EchoStateNetwork ESN = EchoStateNetwork();
//    //float input[20] = {0.5714907356783296,-0.48998162472798623,-0.6924305173690097,0.045713602418041344,0.7350437229186904,-0.19135190070948238,-0.4328778313273105,0.5264213231418309,0.875106597658545,-0.28930070476677905,-0.5353298358107359,-0.3208464528565942,-0.9956752291885742,0.7171889055816869,0.37753383590774026,-0.7105126027978903,0.48283335049186693,-0.09548162337500976,-0.5640076128428777,0.7197587332975541};

//    vector<vector<float>> final_result;
//    vector<float> prediction;

//    for(int i = 0; i < samplesNumber; i++) {
//        prediction = ESN.Predict(test_inputs[i], false);
//        final_result.push_back(prediction);
//        //qDebug() << final_result[i] << "\n";
//    }

//    vector<float> trgt(test_target, test_target + samplesNumber);;
//    vector<float> cwrnn;

//    for(int i = 0; i < samplesNumber; i++) {
//        qDebug() << trgt[i] << ' ';
//    }

//    final_result.push_back(trgt);
//    //final_result.push_back(cwrnn);

//    qDebug() << "Start emitting\n";

//    qRegisterMetaType<vector<vector<float>>>("vector<vector<float>>");

//    QString resultString;

//    QFile plotFile("plotData.csv");
//    QTextStream plotStream(&plotFile);

//    plotFile.open(QIODevice::WriteOnly);

//    for(int i = 0; i < final_result.size(); i++) {
//        for(int j = 0; j < final_result[i].size(); j++) {
//            resultString.append(QString::number(final_result[i][j]));
//            plotFile.write(QString::number(final_result[i][j]).toUtf8());

//            resultString.append(" ");
//            plotFile.write(" ");
//        }
//        resultString.append("\n");
//        plotFile.write("\n");
//    }

//    plotFile.close();

//    emit experimentFinished(resultString);
    //emit experimentFinished(final_result);
    //Q_EMIT experimentFinished(final_result);
}

void Experiment::predictOnline(QString data)
{
    QStringList splitData = data.split(" ");
    splitData.removeLast();
    QString passData = splitData.join(" ");

    EchoStateRunnable *echoThread = new EchoStateRunnable(passData);
    echoThread->setAutoDelete(true);
    QThreadPool::globalInstance()->start(echoThread);

    QObject::connect(echoThread, &EchoStateRunnable::dataPredicted, this, &Experiment::printPredictedData);
    //echoThread->start();

    //qDebug() << "I HAVE DATA: " << splitData << endl;
    //QObject::disconnect(echoThread, &EchoStateThread::dataPredicted, this, &Experiment::printPredictedData);

    //delete echoThread;
}

void Experiment::printPredictedData(QString data)
{

    //qDebug() << "PREDICTED DATA: " << dataSplit << endl;
    //qDebug() << QThreadPool::globalInstance()->activeThreadCount() << endl;


//    high_resolution_clock::time_point currentCall = high_resolution_clock::now();

//    qDebug() << "CALLS printPredictedData: " << currentCall.time_since_epoch().count() << endl;

    Q_EMIT passPredictToMainPlot(data);

}



void Experiment::predictOnlineCW(QString data)
{
    QStringList splitData = data.split(" ");
    splitData.removeLast();
    QString passData = splitData.join(" ");

    EchoStateRunnable *echoThread = new EchoStateRunnable(passData);
    echoThread->setAutoDelete(true);
    QThreadPool::globalInstance()->start(echoThread);

    QObject::connect(echoThread, &EchoStateRunnable::dataPredicted, this, &Experiment::printPredictedData);
    //echoThread->start();

    //qDebug() << "I HAVE DATA: " << splitData << endl;
    //QObject::disconnect(echoThread, &EchoStateThread::dataPredicted, this, &Experiment::printPredictedData);

    //delete echoThread;
}

void Experiment::printPredictedDataCW(QString data)
{

    //qDebug() << "PREDICTED DATA: " << dataSplit << endl;
    //qDebug() << QThreadPool::globalInstance()->activeThreadCount() << endl;

    Q_EMIT passPredictToMainPlot(data);

}

