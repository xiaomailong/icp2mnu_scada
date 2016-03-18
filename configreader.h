#ifndef CONFIGREADER_H
#define CONFIGREADER_H

#include "nodes.h"
#include "alarm.h"
#include <QFile>
#include <QString>
#include "mainwindow.h"



class ConfigReader
{
public:
    ConfigReader();

    void SetConfigFileName(QString fileName);
    bool OpenConfig();
    void SeekStartConfig();
    void CloseConfig();
    bool ReadNextNode(QString &objectName, QString &objectType, QString &IP_addr,
                      uint &port, uint &port_repl, uint &port_local,
                      uint &modbus_start_address, uint &num_float_tags);

    bool ReadNextTrend(QString &objectName, QString &trendName,uint &numInBuff);
    bool ReadNextAlarm(QString &alarmType, QString &alarmExpression, QVector<alarm_expr_member_struct> &alarmVectExprMembers,
                       QString &alarmComparison, float &alarmValue, uint &alarmDelay_s, QString &alarmText);
    bool ReadNextVirtualTag(QString &objectName,uint &numInBuff,QString &virtTagExpression, QVector<virt_expr_member_struct> &virtTagVectExprMembers);

private:
    QFile configFile;
    bool foundSectionNodes;
    bool foundSectionTrends;
    bool foundSectionAlarms;
    bool foundSectionVirtualControllers;
};

#endif // CONFIGREADER_H
