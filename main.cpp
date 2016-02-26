#include "mainwindow.h"
#include <QApplication>
#include <QProxyStyle>
#include <QStyleFactory>
#include <QMessageBox>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);




    //hash

    //if (!MainWindow::CheckHash())
    //{
    //    QMessageBox::information(NULL,"MNU SCADA error","Что-то пошло не так...");
    //    return 0;
    //}


    //style

    //"windows", "motif", "cde", "plastique" and "cleanlooks" "fusion"

    QStyle *style = new QProxyStyle(QStyleFactory::create("fusion"));
    a.setStyle(style);


    MainWindow w;
    w.show();

    return a.exec();
}
