#ifndef ALARM_H
#define ALARM_H

#include <QObject>
#include <QDateTime>
#include <QString>
#include <QColor>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSettings>

#include "autostopthread.h"
#include "tag.h"
#include "logger.h"
#include "odbcdb.h"

class Alarms;

const uint ALARM_MESSAGE_TEXT=128;

using std::list;

enum AlarmLevel
{
   Critical,
   Warning,
   Information
};

enum AlarmCondition
{
    Bigger,
    Less,
    Equal
};

enum AlarmType
{
    OnValueChanged,
    OnQualityChanged
};


struct alarm_expr_member_struct
{
    //QString sign;
    QString objectName;
    uint numInBuff;
};

//сравнение для сортировки alarm_expr_member_struct по убыванию длины имени объекта, лямбду не принимает
//static надо убрать - а то свои копии в каждом модуле
//static bool operator<(const alarm_expr_member_struct &a, const alarm_expr_member_struct &b) { return a.objectName.length() > b.objectName.length(); }
bool operator<(const alarm_expr_member_struct &a, const alarm_expr_member_struct &b);

struct alarm_tag_struct
{
    FloatTag *alarmTag;
    QString alarmType;
    QString alarmExpression;
    QVector<alarm_expr_member_struct> vectAlarmExprMembers;
};

struct alarm_message_struct
{
  int ID; //ID от 1 до alarm_count... - идентификаторы алармов, меньше 0 - команды, пока команда одна = -1 - clear all alarms у клиента
  bool active;
  bool confirmed;
  char text[ALARM_MESSAGE_TEXT];

  AlarmLevel alarmLevel;
  AlarmCondition alarmCondition;
  AlarmType alarmType;
  float alarmValue;
  char DT_activate[20];
  char DT_confirmate[20];
  char DT_deactivate[20];
  int actColor_r;
  int actColor_g;
  int actColor_b;
  int act2Color_r;
  int act2Color_g;
  int act2Color_b;
  int nonactColor_r;
  int nonactColor_g;
  int nonactColor_b;
};




class Alarm : public QObject
{
    Q_OBJECT

private:

    bool preactive;
    bool active;
    bool confirmed;
    QString text;
    AlarmLevel alarmLevel;
    AlarmCondition alarmCondition;
    AlarmType alarmType;
    uint delaySec;

    alarm_message_struct  alarm_message;

public:
    float alarmValue;
    int ID;
    QDateTime dt_preactivated;  //активный, ждем время задержки
    QDateTime dt_activated;
    QDateTime dt_confirmed;
    QDateTime dt_deactivated;
    //Alarm();
    explicit Alarm(Alarms *alarms, int id, AlarmLevel alarmlevel,AlarmType alarmtype,FloatTag *tag,
                 AlarmCondition alarmcondition,float alarmvalue,QString alarmtext, uint alarmdelaySec=0);
    void SetDateTimeOfAlarmActivating();
    void SetDateTimeOfAlarmDeactivating();
    void SetDateTimeOfAlarmConfirming();

    QString GetDateTimeOfAlarmActivating();
    QString GetDateTimeOfAlarmDeactivating();
    QString GetDateTimeOfAlarmConfirming();

    QString GetTextOfAlarm();
    bool IsActive();
    bool IsConfirmed();
    void SetActive(bool newactive);
    void SetConfirmed(bool newconfirmed);
    QColor actColor;
    QColor act2Color;
    QColor nonactColor;
    alarm_message_struct* GetAlarmMessage();

signals:
    void AlarmActivating(Alarm *alarm);
    void AlarmDeactivating(Alarm *alarm);
public slots:
    void TagValueChanged(float newvalue);
    void TagQualityChanged(bool newquality);

};
//=================================================================================


class Alarms : public AutoStopThread
{
    Q_OBJECT

public:
    Alarms();
    virtual ~Alarms();
    void AddAlarm(AlarmLevel alarmlevel, AlarmType alarmType, FloatTag *tag,
                  AlarmCondition alarmcondition, float alarmvalue, QString alarmtext, uint alarmdelaySec=0);
    //overload for comfort usage
    void AddAlarm(QString str_alarmlevel,AlarmType alarmType,FloatTag *tag,QString str_alarmcondition,
                  float alarmvalue,QString alarmtext, uint alarmdelaySec=0);
    QList < Alarm* > allAlarmsList;
    QList < Alarm* > enabledAlarmList;

private:
    static const unsigned int maxEnabledAlarmCount=50;
    int next_alarm_ID;
    void run();
    Logger *logger;
    OdbcDb *alarmDB;
    QTcpServer* m_pAlarmServerSocket;
    QList<QTcpSocket*> m_pAlarmClientSocketList;
    //QTcpSocket* lastClient;

    void UpdateAllAlarmsToAllClients();
    void UpdateAllAlarmsToOneClient(QTcpSocket* pClient);

signals:
    void EnabledAlarmsChanged(QList < Alarm* > *pEnabledAlarmList, bool onlyColorChange);

public slots:
    void AddEnabledAlarm(Alarm *alarm);
    void DeleteEnabledAlarm(Alarm *alarm);
    void AcknowledgeManyAlarms(int alarmscount);
    void AcknowledgeOneAlarm();
    //tcp server
    // Slot to handle disconnected client
    void ClientDisconnected();
    void ClientWrite();

private slots:
    // New client connection
    void NewConnection();
};

//=================================================================================
#endif // ALARM_H
