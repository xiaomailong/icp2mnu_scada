#ifndef MAINWINDOW_H
#define MAINWINDOW_H



#include <QMainWindow>
#include <QMessageBox>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QNetworkInterface>
#include <QCryptographicHash>
#include <QRegExp>
#include <QtScript/QScriptEngine>
#include <QTimer>
#include <QListWidgetItem>

#include "logger.h"
#include "alarm.h"
#include "autostopthread.h"
#include "scadaserver.h"
#include "odbcdb.h"
#include "configreader.h"
#include "nodedataviewer.h"



namespace Ui
{
class MainWindow;
}
//=================================================================================

//=================================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static bool CheckHash();



private:
    Ui::MainWindow *ui;
    Logger *logger;
    ScadaServer *ss;


public slots:
    void TextChanged(int iUzel,QString objectName, QString newText);
    void TextChanged_repl(int iUzel, QString objectName,QString newText);
    void TextSave2LogFile(int iUzel, QString objectName,QString newText);
    void buttonMessagesShow_clicked();
    void alarmsChanged(QList<Alarm *> *pEnabledAlarmList, bool onlyColorChange);
    void pushTestButton();
    void ViewNodeData(QListWidgetItem* item);
    //QString FormattedNodeString(CommonNode *node);
signals:
    void textSave2LogFile(int iUzel, QString objectName, QString newText);
};


//===============================================================



#endif // MAINWINDOW_H
