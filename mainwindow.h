#ifndef MAINWINDOW_H
#define MAINWINDOW_H



#include <QMainWindow>
#include <QTimer>
#include <QListWidgetItem>
#include "alarm.h"
#include "autostopthread.h"


namespace Ui {
class MainWindow;
}
//=================================================================================
class TrendWriterThread: public AutoStopThread
{
      Q_OBJECT
public:
    void run();

};
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
    QTimer timer5s_checkConnectAndSendToClients;
    QTimer timer1s_setAlarmTags;
    TrendWriterThread trendWriterThread;


public slots:
    void TextChanged(int iUzel,QString objectName, QString newText);
    void TextChanged_repl(int iUzel, QString objectName,QString newText);
    void TimerEvent5s();
    void TimerEvent1s_setAlarmsTags();
    void TextSave2LogFile(int iUzel, QString objectName,QString newText);
    void buttonMessagesShow_clicked();
    void alarmsChanged(QList<Alarm *> *pEnabledAlarmList, bool onlyColorChange);
    void pushTestButton();
    void ViewNodeData(QListWidgetItem* item);
signals:
    void textSave2LogFile(int iUzel, QString objectName, QString newText);
};


//===============================================================



#endif // MAINWINDOW_H
