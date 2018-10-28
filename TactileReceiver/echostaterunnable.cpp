#include "echostaterunnable.h"

#include <vector>
#include <QTime>
#include <sys/time.h>
#include <chrono>

QStringList data;
QString resultData;


using namespace std;
using namespace std::chrono;

EchoStateRunnable::EchoStateRunnable(QString dataForPrediction)
{
    data = dataForPrediction.split(" ");
}

void EchoStateRunnable::run()
{
    int inputs_size = 300;
    int output_size = 50;
    float* input = new float[300];

    for(int i = 0; i < data.size(); i++) {
        input[i] = data.at(i).toFloat();
    }

    //TODO: исправить обучение при каждом создании объекта

//    high_resolution_clock::time_point currentCall = high_resolution_clock::now();

//    qDebug() << "CALLS: " << currentCall.time_since_epoch().count() << endl;

    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    EchoStateNetwork ESN = EchoStateNetwork();
    std::vector<float> prediction = ESN.Predict(input, false);

    high_resolution_clock::time_point t2 = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>( t2 - t1 ).count();


    qDebug() << "PREDICTION TIME: " << duration << endl;

    //qDebug() << "RAW PREDICTION: " << prediction << endl;

    //Умножение на 2 из-за нулей
    for(int i = 0; i < (output_size * 2) - 1; i++) {
        if(i % 2 != 0) {
            resultData.append(QString::number(prediction[i]));
            resultData.append(" ");
        }
    }
    resultData.append(QString::number(prediction[output_size*2-1]));

    //qDebug() << "RESULT DATA: " << resultData << endl;




    Q_EMIT dataPredicted(resultData);

    resultData.clear();
}
