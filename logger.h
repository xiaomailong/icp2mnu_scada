#ifndef LOGGER_H
#define LOGGER_H

#include <QDialog>
#include <QColor>
#include <QMutex>
#include <QDateTime>

//#include <list>

//using std::list; -// расскомментировать messageList в .h и .cpp
// если понадобиться  вести лог досоздания
// окна - будет писать в лист, а потом при
// создании окна добавлять в окно
//===============================================================
namespace Ui
{
class LoggerWindow;
}
//===============================================================
class LoggerWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoggerWindow(QWidget *parent = 0);
    ~LoggerWindow();
public slots:
    void AddMessage(QString what, QColor color);
private:
    Ui::LoggerWindow *ui;
    static const int max_messages_count_in_window=5000;
};
//===============================================================
/*
struct Message
{
    QString what;
    QColor color;
};
*/
//=============================================================
class Logger: public QObject
{
    Q_OBJECT
private:
    Logger();
    ~Logger();

    static Logger* theSingleInstanceLogger;
    static LoggerWindow *theSingleInstanceLoggerWindow;
//    static list<Message> messageList;
    static QMutex mutex;

public:
    static Logger* Instance();
    static LoggerWindow* InstanceWindow();
    void AddLog(QString what, QColor color);

signals:
    void signalAddMessage(QString what, QColor color);

};

#endif // LOGGER_H
