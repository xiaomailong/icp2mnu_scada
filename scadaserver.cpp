#include "scadaserver.h"

//выделяем память под синглтон
ScadaServer* ScadaServer::theSingleInstanceScadaServer=NULL;
QMutex ScadaServer::mutex;

//===========================================================
ScadaServer::ScadaServer()
{
    alarms=new Alarms();

    trend_path="d:\\MNU_SCADA\\trends\\";

    for(int i=0;i<17280;i++)
    {
        empty_file[i]=min_float;
    }
}
//===========================================================
ScadaServer::~ScadaServer()
{


    foreach(Alarm* al, alarms->allAlarmsList)
    {
        delete al;
    }

    foreach(alarm_tag_struct alarmTg, vectAlarmTags)
    {
        delete alarmTg.alarmTag;
    }


    delete alarms;




    delete theSingleInstanceScadaServer;
}
//===========================================================
ScadaServer* ScadaServer::GetInstance()
{
    mutex.lock();
    if(theSingleInstanceScadaServer==NULL) theSingleInstanceScadaServer=new ScadaServer();
    mutex.unlock();
    return theSingleInstanceScadaServer;
}
//============================================================
