#include "mainwindow.h"
#include <QApplication>
#include <vector>


// TODO: проверить работает ли
Q_DECLARE_METATYPE(vector<vector<float>>)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;



    w.show();

    //qRegisterMetaType<std::vector<std::vector<float>>>("vector<vector<float>>");
    qRegisterMetaType<vector<vector<float>>>("vector<vector<float>>");

    return a.exec();
}


