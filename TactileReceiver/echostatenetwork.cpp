#include "echostatenetwork.h"
using namespace Eigen;

IOFormat OctaveFmt(StreamPrecision, 0, ", ", ";\n", "", "", "[", "]");
IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");

EchoStateNetwork::EchoStateNetwork()
{
    QFile loadFile("Experiments/ESN300-50-10");
    QTextStream loadStream(&loadFile);

    if(!loadFile.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", loadFile.errorString());
    }

    QStringList inputs = loadStream.readLine().split(" ");
    this->n_inputs = inputs.at(1).toInt();

    //qDebug() << this->n_inputs << '\n';

    QStringList reservoir = loadStream.readLine().split(" ");
    this->n_reservoir = reservoir.at(1).toInt();

    //qDebug() << this->n_reservoir << '\n';

    QStringList outputs = loadStream.readLine().split(" ");
    this->n_outputs = outputs.at(1).toInt();

    //qDebug() << this->n_outputs << '\n';

    QStringList spectralRad = loadStream.readLine().split(" ");
    this->spectral_radius = spectralRad.at(1).toFloat();

    //qDebug() << this->spectral_radius << '\n';

    QStringList sparsity = loadStream.readLine().split(" ");
    this->sparsity = sparsity.at(1).toFloat();

    //qDebug() << this->sparsity << '\n';

    QStringList noise = loadStream.readLine().split(" ");
    this->noise = noise.at(1).toFloat();

    //qDebug() << this->noise << '\n';

    QStringList randomState = loadStream.readLine().split(" ");
    this->randomState = randomState.at(1).toFloat();

    //qDebug() << this->randomState << '\n';

    this->W = new float*[this->n_reservoir];
    this->W_in = new float*[this->n_reservoir];

    //qDebug() << "Size of W: " << this->W << "\n";

    for(int i = 0; i < this->n_reservoir; i++) {
        this->W[i] = new float[this->n_reservoir];
        this->W_in[i] = new float[this->n_inputs];
    }

    //this->W_out = new float[this->n_reservoir + this->n_inputs];
    //___________________________________________________________
    this->W_out = new float*[this->n_outputs];

    for(int i = 0; i < this->n_outputs; i++) {
        this->W_out[i] = new float[this->n_reservoir + this->n_inputs];
    }
    //___________________________________________________________

    // !!!!! * для multiple outputs
    //this->W_feedb = new float[this->n_reservoir * this->n_outputs];
    this->W_feedb = new float*[this->n_reservoir];
    for(int i = 0; i < this->n_reservoir; i++) {
        this->W_feedb[i] = new float[this->n_outputs];
    }
    //_____________________________________________________________

    this->laststate = new float[this->n_reservoir];
    this->lastinput = new float[this->n_inputs];
    this->lastoutput = new float[this->n_outputs];

    if(loadStream.readLine() == "laststate") {
        //qDebug() << "indide laststate\n";
        QStringList tempstate = loadStream.readLine().split(" ");
        for(int i = 0; i < this->n_reservoir; i++) {
            this->laststate[i] = tempstate.at(i).toFloat();
        }
    }

    loadStream.readLine();

    //TODO: убрать создание большого QStringList

    if(loadStream.readLine() == "lastinput") {
        //qDebug() << "indide lastinput\n";
        QStringList tempstate = loadStream.readLine().split(" ");
        for(int i = 0; i < this->n_inputs; i++) {
            this->lastinput[i] = tempstate.at(i).toFloat();
        }
    }

    loadStream.readLine();

    //TODO: переписать lastoutput на случай вектора ОШИБКА в lastoutput

    if(loadStream.readLine() == "lastoutput") {
        //qDebug() << "indide lastoutput\n";
        this->lastoutput[0] = loadStream.readLine().toFloat();
    }

    //qDebug() << "quitted lastoutput\n";

    loadStream.readLine();
    if(loadStream.readLine() == "ESN.W_IN") {
        //qDebug() << "indide ESN.W_IN\n";
        for(int i = 0; i < this->n_reservoir; i++) {
            QStringList tempwin = loadStream.readLine().split(" ");
            for(int j = 0; j < this->n_inputs; j++) {
                this->W_in[i][j] = tempwin.at(j).toFloat();
            }
        }
    }

    loadStream.readLine();
    loadStream.readLine();

    if(loadStream.readLine() == "ESN.W") {
        //qDebug() << "indide ESN.W\n";
        for(int i = 0; i < this->n_reservoir; i++) {
            QStringList tempw = loadStream.readLine().split(" ");
            for(int j = 0; j < this->n_reservoir; j++) {
                this->W[i][j] = tempw.at(j).toFloat();
            }
        }
    }

    //qDebug() << "quitted ESN.W";

    loadStream.readLine();
    loadStream.readLine();

    //Изменено для multiple c *_____________________________________
//    if(loadStream.readLine() == "ESN.W_OUT") {
//        //qDebug() << "indide ESN.W_OUT\n";
//        QStringList tempwout = loadStream.readLine().split(" ");
//        for(int i = 0; i < this->n_reservoir + this->n_inputs; i++) {
//            this->W_out[i] = tempwout.at(i).toFloat();
//        }
//    }
    //______________________________________________________________

    if(loadStream.readLine() == "ESN.W_OUT") {
        for(int i = 0; i < this->n_outputs; i++) {
            QStringList tempwout = loadStream.readLine().split(" ");
            for(int j = 0; j < this->n_reservoir + this->n_inputs; j++)
                this->W_out[i][j] = tempwout.at(j).toFloat();
        }
    }

    loadStream.readLine();
    loadStream.readLine();



    // TODO проверить значения
//    if(loadStream.readLine() == "ESN.W_FEEDB") {
//        //qDebug() << "indide ESN.W_FEEDB\n";
//        QStringList tempwout = loadStream.readLine().split(" ");
//        for(int i = 0; i < this->n_reservoir*this->n_outputs; i++) {
//            this->W_feedb[i] = tempwout.at(i).toFloat();
//        }
//    }

    if(loadStream.readLine() == "ESN.W_FEEDB") {
        //qDebug() << "indide ESN.W_FEEDB\n";
        for(int i = 0; i < this->n_reservoir; i++) {
            QStringList tempwout = loadStream.readLine().split(" ");
            for(int j = 0; j < this->n_outputs; j++) {
                this->W_feedb[i][j] = tempwout.at(j).toFloat();
            }
        }
    }

    loadFile.close();

    // Read test
    for(int i = 0; i < this->n_reservoir; i++) {
        //qDebug() << this->W[i][0];
    }
    // End of read test

    this->W_matx = this->ConvertToEigenMatrix(this->W, this->n_reservoir, this->n_reservoir);
    this->W_in_matx = this->ConvertToEigenMatrix(this->W_in, this->n_reservoir, this->n_inputs);
    //this->W_out_matx = Map<MatrixXf>(this->W_out, this->n_reservoir, this->n_outputs);
    this->W_out_matx = this->ConvertToEigenMatrix(this->W_out, this->n_outputs, this->n_reservoir + this->n_inputs);
    //this->W_out_matx = Map<MatrixXf>(this->W_out, 1, this->n_reservoir + this->n_inputs);
    //this->W_feedb_matx = Map<VectorXf>(this->W_feedb, this->n_reservoir * this->n_outputs);
    this->W_feedb_matx = this->ConvertToEigenMatrix(this->W_feedb, this->n_reservoir, this->n_outputs);

    //qDebug() << "W_matx [" << this->W_matx.rows() << "," << this->W_matx.cols() << "]\n";
    //qDebug() << "W_in_matx [" << this->W_in_matx.rows() << "," << this->W_in_matx.cols() << "]\n";
    //qDebug() << "W_out_matx [" << this->W_out_matx.rows() << "," << this->W_out_matx.cols() << "]\n";
    //qDebug() << "W_feedb_matx [" << this->W_feedb_matx.rows() << "," << this->W_feedb_matx.cols() << "]\n";

}

EchoStateNetwork::~EchoStateNetwork()
{
    for(int i = 0; i < this->n_reservoir; i++) {
        delete[] W[i];
        delete[] W_in[i];
    }
    delete[] W;
    delete[] W_in;
    delete[] W_out;
    delete[] W_feedb;
}

VectorXf EchoStateNetwork::Update(VectorXf state_vec, VectorXf input_vec, VectorXf output_vec)
{

    //VectorXf state_vec = Map<VectorXf>(state, 1, sizeof(state));
    //VectorXf input_vec = Map<VectorXf>(input, 1, sizeof(input));
    //VectorXf output_vec = Map<VectorXf>(output, 1, sizeof(output));

    VectorXf preactivation;

    if(this->teacher_forcing == true) {
        //qDebug() << "Started mult\n";
        preactivation = this->W_matx*state_vec + this->W_in_matx*input_vec + this->W_feedb_matx*output_vec;
    } else {
        preactivation = this->W_matx*state_vec + this->W_in_matx*input_vec;
    }

    //qDebug() << "ended mult\n";

    VectorXf randVec = VectorXf::Random(this->n_reservoir);
    VectorXf result = tanh(preactivation.array()) + this->noise * ((randVec.array()+1)/2 - 0.5);

    //qDebug() << "ended UPDATE FUNC\n";

    return result;

}

MatrixXf EchoStateNetwork::ScaleInputs(MatrixXf inputs)
{
    if(this->inputScaling != -1) {
        inputs = inputs * VectorXf::Constant(inputs.rows(), this->inputScaling).asDiagonal();
    }

    if(this->inputShift != 0) {
        inputs = inputs.array() + this->inputShift;
    }

    return inputs;
}

MatrixXf EchoStateNetwork::UnscaleTeacher(MatrixXf teacher_scaled)
{
    if(this->teacherShift != 0) {
        teacher_scaled = teacher_scaled.array() - this->teacherShift;
    }

    if(this->teacherScaling != -1) {
        teacher_scaled = teacher_scaled / this->teacherScaling;
    }

    return teacher_scaled;
}

std::vector<float> EchoStateNetwork::Predict(float *inputs_in, bool continuation)
{
    // Входная строка правильно (вертикально)

    MatrixXf inputs_vec = Map<MatrixXf>(inputs_in, 1, this->n_inputs);

    //int n_samples = inputs_vec.cols();
    int n_samples = inputs_vec.rows();

    //qDebug() << "N SAMPLES: " << n_samples << endl;

    MatrixXf laststate_vec;
    MatrixXf lastinput_vec;
    MatrixXf lastoutput_vec;

    if(continuation == true) {
        laststate_vec = Map<MatrixXf>(this->laststate, this->n_reservoir, 1);
        lastinput_vec = Map<MatrixXf>(this->lastinput, this->n_inputs, 1);
        lastoutput_vec = Map<MatrixXf>(this->lastoutput, this->n_outputs, 1);
    } else {

        laststate_vec = MatrixXf::Zero(1, this->n_reservoir);
        lastinput_vec = MatrixXf::Zero(1, this->n_inputs);
        lastoutput_vec = MatrixXf::Zero(1, this->n_outputs);
    }

    //qDebug() << "Checked continuation\n";

//    std::cout << "LASTSTATE\n";
//    std::cout << laststate_vec.format(CleanFmt) << '\n';
//    std::cout << "LASTINPUT\n";
//      std::cout << lastinput_vec.format(CleanFmt) << '\n';
//      std::cout << inputs_vec.format(CleanFmt) << '\n';
//    std::cout << "LASTOUTPUT\n";
//    std::cout << lastoutput_vec.format(CleanFmt) << '\n';

    MatrixXf inputs(lastinput_vec.rows() + inputs_vec.rows(), this->n_inputs);
    inputs << lastinput_vec, inputs_vec;

    MatrixXf nullmtx = MatrixXf::Zero(n_samples, this->n_reservoir);

    MatrixXf states(laststate_vec.rows() + n_samples, this->n_reservoir);
    states << laststate_vec, nullmtx;

    //qDebug() << "passed states\n";

    // TODO: Не обязательно
    MatrixXf nullmtx2 = MatrixXf::Zero(n_samples, this->n_outputs);

    MatrixXf outputs(lastoutput_vec.rows() + n_samples, this->n_outputs);
    outputs << lastoutput_vec, nullmtx2;

    //qDebug() << "passed outputs\n";


    for(int i = 0; i < n_samples; i++) {
        //qDebug() << "COL INPUTS SIZE: " << inputs.row(i).size() << '\n';

        states.row(i+1) = this->Update(states.row(i), inputs.row(i+1), outputs.row(i));
        //states.col(i+1) = this->Update(states.col(i), inputs.col(i+1), outputs.col(i));

        //qDebug() << "VEC JOINED STARTED\n";

        MatrixXf vec_joined(this->n_reservoir + this->n_inputs, 1);

        // До этого правильно


        vec_joined << states.transpose().col(i+1), inputs.transpose().col(i+1);

        //qDebug() << "VEC JOINED CREATED\n";
        //qDebug() << "LAST OPERATION\n";

        //qDebug() << "LAST OP ROWS COLS" << this->W_out_matx.rows() << " " << this->W_out_matx.cols() << "    " << vec_joined.rows() << " " << vec_joined.cols();

        //outputs.row(i+1) = this->W_out_matx * vec_joined;

        // !!! Добавление для multiple
        for(int j = 0; j < n_outputs; j++) {
            outputs.row(i+1).col(j) = this->W_out_matx.row(j) * vec_joined;
        }

        //qDebug() << "END OF LAST OPERATION\n";
    }

    //std::cout << outputs.format(CleanFmt) << '\n';

    //float* returnArray[outputs.cols()];
    std::vector<float> vec(outputs.size());
    Map<MatrixXf>(vec.data(), outputs.rows(), outputs.cols()) = outputs;
    //qDebug() << vec << endl;
    //std::cout << vec[1] << '\n';

    //std::cout << "Vec size " << vec.size() << "\n";
            //this->UnscaleTeacher(outputs); // !! output[1:]
    return vec;

}

MatrixXf EchoStateNetwork::ConvertToEigenMatrix(float** data, int rowNum, int colNum)
{
    Eigen::MatrixXf eMatrix(rowNum, colNum);
    for (int i = 0; i < rowNum; ++i)
        eMatrix.row(i) = Eigen::VectorXf::Map(&data[i][0], colNum);
    return eMatrix;
}
