#ifndef NODES_H
#define NODES_H

#include <QTimer>
#include <QTime>
#include <QFile>
#include <QtScript/QScriptEngine>

#include "mainwindow.h"
#include "autostopthread.h"
#include "srvretranslater.h"
#include "logger.h"
#include "./src_libmodbus/modbus.h"
//==============================================================
struct virt_expr_member_struct
{
QString objectName;
uint numInBuff;
};

//сравнение для сортировки по убыванию длины имени объекта, лямбду не принимает
bool operator<(const virt_expr_member_struct &a, const virt_expr_member_struct &b);

struct virt_tag_struct
{
uint numInBuff;
QString virtTagExpression;
QVector<virt_expr_member_struct> vectVirtTagExprMembers;
};

//==============================================================
class CommonTrend
{
public:
    CommonTrend(QString objectName,QString trendName,uint numInBuff)
    {
        m_objectName=objectName;
        m_trendName=trendName;
        m_numInBuff=numInBuff;
    }
    QString m_objectName;
    QString m_trendName;
    uint m_numInBuff;
};

enum NodeType
{
    MODBUS=0,
    MNU_SCADA
};
//==============================================================
class CommonNode: public AutoStopThread
{
    Q_OBJECT
public:

    CommonNode();
    virtual ~CommonNode();

    static CommonNode* CreateNode(MainWindow *mw ,QString objectName,QString objectType,
                                  QString IP_addr, uint port,
                                  uint port_repl,uint port_local,
                                  uint modbus_start_address,
                                  uint num_float_tags);

    static uint nodes_counter;

    QString m_IP_addr;
    unsigned int m_port;
    unsigned int m_port_repl;
    QString m_nameObject;
    QString m_typeObject;   //modbus or mnu_scada or virtual

    QString m_text_client;
    QString m_text_repl;

    unsigned int m_modbus_start_address;

    bool m_isConnected;
    bool m_isReaded;
    uint m_num_reads;
    uint m_sec_counter;

    unsigned int m_this_number;

    SrvReTranslater m_srv;
    unsigned int m_port_local;

    QString FormattedNodeString();

signals:
    void textchange(int iUzel,QString objectName, QString newText);
    void textchange_repl(int iUzel,QString objectName, QString newText);
    void textSave2LogFile(int iUzel,QString objectName, QString newText);

};
//==============================================================

class ModbusNode: public CommonNode
{
      Q_OBJECT
public:
      explicit ModbusNode(int this_number,QString objectName,QString objectType,
                          QString IP_addr, uint port,
                          uint port_repl,uint port_local,
                          uint modbus_start_address,
                          uint num_float_tags);
      virtual ~ModbusNode() {;}
      virtual void run();   // poll loop thread
};


//============================================================
class MnuScadaNode : public CommonNode
{
    Q_OBJECT
public:
    explicit MnuScadaNode(int this_number,QString objectName,QString objectType,
                           QString IP_addr, uint port,
                           uint port_repl,uint port_local,
                           uint modbus_start_address,
                           uint num_float_tags);
    virtual ~MnuScadaNode();

    QTcpSocket *socket;
    QTimer timerConnectLoop;

    virtual void run();  //trend recovery thread

    void doConnect();
    bool getStateConnected();
    bool getStateConnecting();

    bool need_restart_repl;

public slots:
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError er);
    void bytesWritten(qint64 bytes);
    void readyRead();

    void TimerConnectLoopEvent();



};
//===============================================================
//Виртуальный контроллер на сервере
//производит рассчет величин на основе полученных с удаленных узлов
//к нему можно подключиться по протоколу MADBUS
class VirtualNode: public CommonNode
{
      Q_OBJECT
public:
      explicit VirtualNode(int this_number,QString objectName,QString objectType,
                          QString IP_addr, uint port,
                          uint port_repl,uint port_local,
                          uint modbus_start_address,
                          uint num_float_tags);
      virtual ~VirtualNode();

      QVector<virt_tag_struct> vectVirtTags;

private:
      QTimer timerCalculateVariables;
      QScriptEngine virtControllerScriptEngine;

public slots:
      void TimerCalculateVariablesEvent();
//      virtual void run();   //do not need, QTimer is better
};
//================================================================




#endif // NODES_H
