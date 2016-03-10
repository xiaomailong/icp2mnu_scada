#include "alarm.h"



//=================================================================
bool operator<(const alarm_expr_member_struct &a, const alarm_expr_member_struct &b)
{
    return a.objectName.length() > b.objectName.length();
}
//==================================================================
bool compare_alarms(Alarm *first,Alarm *second)
{
    if (!second->IsConfirmed() && first->IsConfirmed()) return false;
    if (!first->IsConfirmed() && second->IsConfirmed()) return true;
    return (first->dt_activated >= second->dt_activated);
}

//=================================================================
bool Alarm::IsActive()
{
    return active;
}
//=================================================================
bool Alarm::IsConfirmed()
{
    return confirmed;
}
//=================================================================
void Alarm::SetActive(bool newactive)
{
    active=newactive;
}
//=================================================================
void Alarm::SetConfirmed(bool newconfirmed)
{
    confirmed=newconfirmed;

    if (confirmed==true)
    {
        SetDateTimeOfAlarmConfirming();
    }

    if (confirmed==true && active==false)  // 5.11.2015
    {
        emit AlarmDeactivating(this);
    }

}
//=================================================================
QString Alarm::GetTextOfAlarm()
{
    if (delaySec>0)
    {
        return text+" ("+QString::number(delaySec)+"s)";
    }
    else
    {
        return text;
    }
}
//=================================================================
QString Alarm::GetDateTimeOfAlarmActivating()
{
    QString tmp;
    tmp.sprintf("%.2u.%.2u.%.4u %.2u:%.2u:%.2u",
                dt_activated.date().day(),dt_activated.date().month(),dt_activated.date().year(),
                dt_activated.time().hour(),dt_activated.time().minute(),dt_activated.time().second());
    return tmp;
}
//=================================================================
QString Alarm::GetDateTimeOfAlarmDeactivating()
{
    QString tmp;
    tmp.sprintf("%.2u.%.2u.%.4u %.2u:%.2u:%.2u",
                dt_deactivated.date().day(),dt_deactivated.date().month(),dt_deactivated.date().year(),
                dt_deactivated.time().hour(),dt_deactivated.time().minute(),dt_deactivated.time().second());
    return tmp;
}
//=================================================================
QString Alarm::GetDateTimeOfAlarmConfirming()
{
    QString tmp;
    tmp.sprintf("%.2u.%.2u.%.4u %.2u:%.2u:%.2u",
                dt_confirmed.date().day(),dt_confirmed.date().month(),dt_confirmed.date().year(),
                dt_confirmed.time().hour(),dt_confirmed.time().minute(),dt_confirmed.time().second());
    return tmp;
}
//=================================================================
void Alarm::SetDateTimeOfAlarmActivating()
{
    dt_activated=QDateTime::currentDateTime();
}
//=================================================================
void Alarm::SetDateTimeOfAlarmDeactivating()
{
    dt_deactivated=QDateTime::currentDateTime();
}
//=================================================================
void Alarm::SetDateTimeOfAlarmConfirming()
{
    dt_confirmed=QDateTime::currentDateTime();
}
//=================================================================
Alarm::Alarm(Alarms *alarms, int id, AlarmLevel alarmlevel, AlarmType alarmtype, FloatTag *tag, AlarmCondition alarmcondition,
             float alarmvalue, QString alarmtext, uint alarmdelaySec)
{
    ID=id;
    preactive=false;
    active=false;
    confirmed=false;
    alarmlevel=alarmlevel;
    alarmCondition=alarmcondition;
    alarmValue=alarmvalue;
    text=alarmtext;
    delaySec=alarmdelaySec;

    if (alarmlevel==Critical)
    {
        actColor.setRgb(255,0,0);
        nonactColor.setRgb(177,182,184);
        act2Color.setRgb(255,128,128);
    }
    if (alarmlevel==Warning)
    {
        actColor.setRgb(255,255,0);
        nonactColor.setRgb(177,182,184);
        act2Color.setRgb(177,182,184);
    }

    if (alarmlevel==Information)
    {
        actColor.setRgb(31,84,224);
        nonactColor.setRgb(177,182,184);
        act2Color.setRgb(143,170,239);
    }

    if (alarmtype==OnValueChanged)
        connect(tag,SIGNAL(ValueChanged(float)),this,SLOT(TagValueChanged(float)));

    if (alarmtype==OnQualityChanged)
        connect(tag,SIGNAL(QualityChanged(bool)),this,SLOT(TagQualityChanged(bool)));

    connect(this,SIGNAL(AlarmActivating(Alarm*)),alarms,SLOT(AddEnabledAlarm(Alarm*)));
    connect(this,SIGNAL(AlarmDeactivating(Alarm*)),alarms,SLOT(DeleteEnabledAlarm(Alarm*)));
}
//=================================================================
void Alarm::TagValueChanged(float newvalue)
{


    if ((preactive==false && alarmCondition==Bigger && newvalue>alarmValue  )   ||
        (preactive==false && alarmCondition==Less   && newvalue<alarmValue  )   ||
        (preactive==false && alarmCondition==Equal  && newvalue==alarmValue ))
    {
        //SetDateTimeOfAlarmActivating();
        dt_preactivated=QDateTime::currentDateTime();
        preactive=true;
    }

    if ((preactive==true && alarmCondition==Bigger && newvalue<=alarmValue  )   ||
        (preactive==true && alarmCondition==Less   && newvalue>=alarmValue  )   ||
        (preactive==true && alarmCondition==Equal  && newvalue!=alarmValue ))
    {
        preactive=false;
    }




    if ((preactive==true && active==false &&  dt_preactivated.secsTo(QDateTime::currentDateTime())>=delaySec && alarmCondition==Bigger && newvalue>alarmValue  )   ||
        (preactive==true && active==false &&  dt_preactivated.secsTo(QDateTime::currentDateTime())>=delaySec && alarmCondition==Less && newvalue<alarmValue  )   ||
        (preactive==true && active==false &&  dt_preactivated.secsTo(QDateTime::currentDateTime())>=delaySec && alarmCondition==Equal  && newvalue==alarmValue ))
    {
        SetDateTimeOfAlarmActivating();
        active=true;
  //      if (confirmed==true)
  //      {

           confirmed=false;

   //     oldalarms.AddAlarm(this);
        emit AlarmActivating(this);
  //      }

    }

    if ((active==true && alarmCondition==Bigger && newvalue<=alarmValue) ||
        (active==true && alarmCondition==Less   && newvalue>=alarmValue) ||
        (active==true && alarmCondition==Equal  && newvalue!=alarmValue))

    {
        preactive=false;
        active=false;
        //confirmed=false;   // commented 5.11.2015
        SetDateTimeOfAlarmDeactivating();
        emit AlarmDeactivating(this);
    }

}
//=================================================================
void Alarm::TagQualityChanged(bool newquality)
{
float fltquality;
    if (newquality) fltquality=1.0;
    else fltquality=0.0;

    if ((active==false && alarmCondition==Bigger && fltquality>alarmValue  )   ||
        (active==false && alarmCondition==Less   && fltquality<alarmValue  )   ||
        (active==false && alarmCondition==Equal  && fltquality==alarmValue ))
    {
        SetDateTimeOfAlarmActivating();
        active=true;
  //      if (confirmed==true)
  //      {

        confirmed=false;

   //     oldalarms.AddAlarm(this);
        emit AlarmActivating(this);
  //      }

    }

    if ((active==true && alarmCondition==Bigger && fltquality<=alarmValue) ||
        (active==true && alarmCondition==Less   && fltquality>=alarmValue) ||
        (active==true && alarmCondition==Equal  && fltquality!=alarmValue))

    {
        active=false;
        confirmed=false;
        SetDateTimeOfAlarmDeactivating();
        emit AlarmDeactivating(this);
    }

}
//=================================================================
alarm_message_struct* Alarm::GetAlarmMessage()
{
    /*
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

    };
    */

   alarm_message.ID=this->ID;
   alarm_message.active=this->IsActive();
   alarm_message.confirmed=this->IsConfirmed();
   snprintf(alarm_message.text,ALARM_MESSAGE_TEXT,"%s ",this->GetTextOfAlarm().toStdString().c_str());
   alarm_message.alarmLevel=this->alarmLevel;
   alarm_message.alarmCondition=this->alarmCondition;
   alarm_message.alarmType=this->alarmType;
   alarm_message.alarmValue=this->alarmValue;
   snprintf(alarm_message.DT_activate,20,"%s",this->GetDateTimeOfAlarmActivating().toStdString().c_str());
   snprintf(alarm_message.DT_deactivate,20,"%s",this->GetDateTimeOfAlarmDeactivating().toStdString().c_str());
   snprintf(alarm_message.DT_confirmate,20,"%s",this->GetDateTimeOfAlarmConfirming().toStdString().c_str());

   this->actColor.getRgb(&(alarm_message.actColor_r),&(alarm_message.actColor_g),&(alarm_message.actColor_b));
   this->act2Color.getRgb(&(alarm_message.act2Color_r),&(alarm_message.act2Color_g),&(alarm_message.act2Color_b));
   this->nonactColor.getRgb(&(alarm_message.nonactColor_r),&(alarm_message.nonactColor_g),&(alarm_message.nonactColor_b));

   return &alarm_message;

}
//=================================================================
Alarms::Alarms()
{
next_alarm_ID=0;
logger=Logger::Instance();
enabledAlarmList.clear();

alarmDB=new OdbcDb("fire_alarmdb","SYSDBA","784523");
alarmDB->AddAlarm2DB("Alarms Server Started...");

logger->AddLog("АЛАРМ: Старт подсистемы...",Qt::darkCyan);

//tcp server
m_pAlarmServerSocket = new QTcpServer(this);
m_pAlarmServerSocket->listen(QHostAddress::Any, 7000);

connect(m_pAlarmServerSocket, SIGNAL(newConnection()), this, SLOT(NewConnection()));

}
//=================================================================
Alarms::~Alarms()
{
   alarmDB->AddAlarm2DB("Alarms Server Closed...");
   delete alarmDB;
   delete m_pAlarmServerSocket;
}
//=================================================================

void Alarms::NewConnection()
{
    QTcpSocket* pClient = m_pAlarmServerSocket->nextPendingConnection();
    pClient->setSocketOption(QAbstractSocket:: KeepAliveOption, 1);
    m_pAlarmClientSocketList.push_back(pClient);

    //TO DO: send alarms
    UpdateAllAlarmsToOneClient(pClient);

    connect(pClient, SIGNAL(disconnected()), this, SLOT(ClientDisconnected()));
    connect(pClient,SIGNAL(readyRead()),this,SLOT(ClientWrite()));
    //lastClient=pClient;

    logger->AddLog("АЛАРМ: К серверу алармов подключился "+pClient->peerAddress().toString(),Qt::darkCyan);
    alarmDB->AddAlarm2DB("До сервера алармів під`єднався "+pClient->peerAddress().toString());

}
//======================================================================================
void Alarms::ClientDisconnected()
{
    // client has disconnected, so remove from list
    QTcpSocket* pClient = static_cast<QTcpSocket*>(QObject::sender());
    m_pAlarmClientSocketList.removeOne(pClient);
    logger->AddLog("АЛАРМ: От сервера алармов отключился "+pClient->peerAddress().toString(),Qt::darkCyan);
    alarmDB->AddAlarm2DB("Від сервера алармів від`єднався "+pClient->peerAddress().toString());

}
//===========================================================================================================
void Alarms::ClientWrite()
{
    QTcpSocket* pClient = static_cast<QTcpSocket*>(QObject::sender());

    qDebug() << "reading form client..."+QString::number(pClient->bytesAvailable());
    alarm_message_struct alarm_message;

    // read the data from the socket
    while (pClient->bytesAvailable() >= sizeof(alarm_message_struct))
    {
        pClient->read((char *)&alarm_message,sizeof(alarm_message_struct));
        //найдем аларм по ИД и подтверждаем его:
        foreach(Alarm *alarm, enabledAlarmList)
        {
            if (alarm->ID==alarm_message.ID)
            {
                alarm->SetConfirmed(true);
                logger->AddLog("АЛАРМ: Квитирован по сети с "+pClient->peerAddress().toString()+" :"+alarm->GetTextOfAlarm(),Qt::darkCyan);
                alarmDB->AddAlarm2DB("Квитація з "+pClient->peerAddress().toString()+" :"+alarm->GetTextOfAlarm() +
                                     "  ("+QString::number(alarm->alarmValue,'f',2)+")");

                qSort(enabledAlarmList.begin(),enabledAlarmList.end(),compare_alarms);
                emit EnabledAlarmsChanged(&enabledAlarmList, false);
                UpdateAllAlarmsToAllClients();
                break; //по ходу он должен быть первым (не всегда, задержки сети могут помешать)
            }
        }

        qDebug() << alarm_message.ID << "  " << alarm_message.text;

    }
    if (pClient->bytesAvailable()>0 && pClient->bytesAvailable()<sizeof(alarm_message_struct))  //осталось байт не кратное одной посылке, - херня,ошибка, не тот сервер
    {
        pClient->disconnectFromHost();
    }

}
//====================================================================================
void Alarms::UpdateAllAlarmsToOneClient(QTcpSocket* pClient)
{
alarm_message_struct erase_message;
erase_message.ID=-1;
erase_message.text[0]=0;

        pClient->write((char *)&erase_message,sizeof(alarm_message_struct));

        qDebug() << "writing to alarm =" << sizeof(alarm_message_struct);

        foreach(Alarm *alarm, enabledAlarmList)
        {
            pClient->write((char *)alarm->GetAlarmMessage(),sizeof(alarm_message_struct));
            pClient->waitForBytesWritten(5000);
        }
}
//=================================================================
void Alarms::UpdateAllAlarmsToAllClients()
{
    foreach(QTcpSocket* pClient, m_pAlarmClientSocketList)
    {
        UpdateAllAlarmsToOneClient(pClient);
    }
}
//=================================================================

void Alarms::AddAlarm(AlarmLevel alarmlevel,AlarmType alarmType,FloatTag *tag,AlarmCondition alarmcondition,
              float alarmvalue,QString alarmtext, uint alarmdelaySec)
{
    next_alarm_ID++;
    Alarm *newAlarm=new Alarm(this, next_alarm_ID, alarmlevel,alarmType,tag,alarmcondition, alarmvalue,alarmtext,alarmdelaySec);
    allAlarmsList.push_back(newAlarm);
    logger->AddLog("АЛАРМ: Добавлен: "+alarmtext+", value="+QString::number(alarmvalue)+" ,delay="+QString::number(alarmdelaySec),Qt::darkCyan);

}
//=================================================================
//overload for comfort usage
void Alarms::AddAlarm(QString str_alarmlevel, AlarmType alarmType, FloatTag *tag, QString str_alarmcondition,
              float alarmvalue, QString alarmtext, uint alarmdelaySec)
{
    AlarmLevel alarmlevel=Information;
    AlarmCondition alarmcondition=Less;

    if (str_alarmlevel=="information") alarmlevel=Information;
    if (str_alarmlevel=="warning") alarmlevel=Warning;
    if (str_alarmlevel=="critical") alarmlevel=Critical;

    if (str_alarmcondition=="<") alarmcondition=Less;
    if (str_alarmcondition=="=") alarmcondition=Equal;
    if (str_alarmcondition==">") alarmcondition=Bigger;

    AddAlarm(alarmlevel,alarmType,tag,alarmcondition,alarmvalue,alarmtext,alarmdelaySec);

}
//=================================================================
void Alarms::AddEnabledAlarm(Alarm *alarm)
{
    if (enabledAlarmList.indexOf(alarm)!=-1) enabledAlarmList.removeOne(alarm);   // 5.11.2015

    if (enabledAlarmList.size()<maxEnabledAlarmCount)
    {
        enabledAlarmList.push_front(alarm);
    }
    else
    {
        enabledAlarmList.pop_back();
        enabledAlarmList.push_front(alarm);
    }
    if (!isRunning()) start();
    emit EnabledAlarmsChanged(&enabledAlarmList, false);
    logger->AddLog("АЛАРМ: Сработал: "+alarm->GetTextOfAlarm(),Qt::darkCyan);
    alarmDB->AddAlarm2DB("Активний: "+alarm->GetTextOfAlarm()+
                         "  ("+QString::number(alarm->alarmValue,'f',2)+")");
    UpdateAllAlarmsToAllClients();
}
//==================================================================
void Alarms::DeleteEnabledAlarm(Alarm *alarm)
{
    if (alarm->IsConfirmed())   // 5.11.2015
    {
    enabledAlarmList.removeOne(alarm);
    }
    emit EnabledAlarmsChanged(&enabledAlarmList, false);
    logger->AddLog("АЛАРМ: Пропал: "+alarm->GetTextOfAlarm(),Qt::darkCyan);
    alarmDB->AddAlarm2DB("Неактивний: "+alarm->GetTextOfAlarm()+
                         "  ("+QString::number(alarm->alarmValue,'f',2)+")");
    UpdateAllAlarmsToAllClients();
}
//==================================================================
void Alarms::AcknowledgeManyAlarms(int count)
{

    QList<Alarm*>::iterator al_iter_begin;
    QList<Alarm*>::iterator al_iter_end;

    if (enabledAlarmList.size()>0)
    {
    al_iter_begin=enabledAlarmList.begin();
    al_iter_end=enabledAlarmList.end();

    for(int i=0;i<count;++i)
    {
    if (al_iter_begin!=al_iter_end)
    {
        if (!(*al_iter_begin)->IsConfirmed())
        {
        (*al_iter_begin)->SetConfirmed(true);
        logger->AddLog("АЛАРМ: Квитирован на сервере: "+(*al_iter_begin)->GetTextOfAlarm(),Qt::darkCyan);
        alarmDB->AddAlarm2DB("Квитований з сервера: "+(*al_iter_begin)->GetTextOfAlarm() +
                             "  ("+QString::number((*al_iter_begin)->alarmValue,'f',2)+")");
        }
        al_iter_begin++;
    }
    else break;
    }
    }
}


//==================================================================
void Alarms::AcknowledgeOneAlarm()
{
    if (enabledAlarmList.size()>0)
    {
        if( !(enabledAlarmList.at(0)->IsConfirmed()) )
        {
            AcknowledgeManyAlarms(1);
            qSort(enabledAlarmList.begin(),enabledAlarmList.end(),compare_alarms);
            //enabledAlarmList.sort(compare_alarms);
            emit EnabledAlarmsChanged(&enabledAlarmList, false);
            UpdateAllAlarmsToAllClients();
        }
    }
}
//==================================================================
void Alarms::run()
{
    bool isBlinking=true;

    while (isBlinking)
    {
        isBlinking=false;
        msleep(500);
        if (CheckThreadStop()) return;
        foreach (Alarm *alarm, enabledAlarmList)
        {
            if (alarm->IsActive() && !alarm->IsConfirmed()) isBlinking=true;
        }
        if (isBlinking) emit EnabledAlarmsChanged(&enabledAlarmList, true);
    }
}
