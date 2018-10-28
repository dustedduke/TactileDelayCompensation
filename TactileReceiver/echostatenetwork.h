#ifndef ECHOSTATENETWORK_H
#define ECHOSTATENETWORK_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include <iostream>
#include <vector>
#include <Eigen/Dense>

using namespace Eigen;

class EchoStateNetwork
{
public:
    int n_inputs;
    int n_outputs;
    int n_reservoir;

    float spectral_radius;
    float sparsity;
    float noise;
    float randomState;

    float inputScaling = -1;
    float inputShift = 0;
    float teacherShift = 0;
    float teacherScaling = -1;

    bool teacher_forcing = true;

    float** W;
    float** W_in;
    //Изменено для multiple с *
    float** W_out;
    //float* W_feedb;
    float** W_feedb;

    float* laststate;
    float* lastinput;
    float* lastoutput;

    MatrixXf W_matx;
    MatrixXf W_in_matx;
    MatrixXf W_out_matx;
    //VectorXf W_feedb_matx;
    MatrixXf W_feedb_matx;

    EchoStateNetwork();
    ~EchoStateNetwork();
    std::vector<float> Predict(float *input, bool continuation);
    VectorXf Update(VectorXf state, VectorXf input, VectorXf output);

    void OutActivation();

    MatrixXf ScaleInputs(MatrixXf inputs);
    MatrixXf UnscaleTeacher(MatrixXf teacher_scaled);

    MatrixXf ConvertToEigenMatrix(float** data, int rowNum, int colNum);
};

#endif // ECHOSTATENETWORK_H
