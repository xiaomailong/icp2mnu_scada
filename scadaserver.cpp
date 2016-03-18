#include "scadaserver.h"

//выделяем память под синглтон
ScadaServer* ScadaServer::theSingleInstanceScadaServer=NULL;
QMutex ScadaServer::mutex;

//===========================================================
ScadaServer::ScadaServer()
{
    alarms=new Alarms();

    logger=Logger::Instance();

    trend_path="d:\\MNU_SCADA\\trends\\";

    for(int i=0; i<17280; i++)
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
ScadaServer* ScadaServer::Instance()
{
    mutex.lock();
    if(theSingleInstanceScadaServer==NULL) theSingleInstanceScadaServer=new ScadaServer();
    mutex.unlock();
    return theSingleInstanceScadaServer;
}
//============================================================
void ScadaServer::StartServer()
{
    connect(&timer5s_checkConnectAndSendToClients,SIGNAL(timeout()),this,SLOT(TimerEvent5s_checkConnectAndSendToClients()));
    timer5s_checkConnectAndSendToClients.start(5000);

    trendWriterThread = new TrendWriter();
    trendWriterThread->start();

    connect(&timer1s_setAlarmTags,SIGNAL(timeout()),this,SLOT(TimerEvent1s_setAlarmsTags()));
    timer1s_setAlarmTags.start(1000);
}
//============================================================
void ScadaServer::StopServer()
{
    //останавливаем таймер пересоединения и отправки пакетов данных
    timer5s_checkConnectAndSendToClients.stop();

    //останавливаем таймеры алармов
    timer1s_setAlarmTags.stop();

    //останавливаем поток записи трендов
    delete trendWriterThread;

    //останавливаем потоки опроса узлов
    foreach(CommonNode* node,hashCommonNodes)
    {
        delete node;
    }

    // и далее все удаляем
    hashCommonNodes.clear();

    foreach(CommonTrend* trend,vectCommonTrends)
    {
        delete trend;
    }

    vectCommonTrends.clear();

}
//============================================================
void ScadaServer::TimerEvent5s_checkConnectAndSendToClients()
{

    foreach(CommonNode* node, hashCommonNodes)
    {
        if (node->m_isReaded &&
                !node->m_srv.m_pServerSocket->isListening())
        {
            node->m_srv.m_pServerSocket->listen(QHostAddress::Any, node->m_port_local);
        }

        if( node->m_isReaded &&
                node->m_srv.m_pServerSocket->isListening())
        {
            for(int j=0; j<node->m_srv.m_pClientSocketList.size(); j++)
            {
                node->m_srv.m_pClientSocketList.at(j)->write((char *)node->m_srv.buff,node->m_srv.num_float_tags*4);
            }
        }


        if( !node->m_isReaded &&
                node->m_srv.m_pServerSocket->isListening())
        {
            for(int j=0; j<node->m_srv.m_pClientSocketList.size(); j++)
            {
                node->m_srv.m_pClientSocketList.at(j)->close();
            }

            node->m_srv.m_pServerSocket->close();
            node->m_srv.m_pClientSocketList.clear();
        }
    }
}
//============================================================
void ScadaServer::TimerEvent1s_setAlarmsTags()
{

    static QScriptEngine alarmScriptEngine;

    foreach(alarm_tag_struct alarmDescStruct, vectAlarmTags)
    {
        if (alarmDescStruct.alarmType=="connect")
        {
            alarmDescStruct.alarmTag->SetValueQuality(hashCommonNodes[alarmDescStruct.alarmExpression]->m_isConnected,true);
            // logger->AddLog("Changed Alarm: "+alarmDescStruct.alarmType+"  " + alarmDescStruct.alarmExpression+"  val="+QString::number(hashCommonNodes[alarmDescStruct.alarmExpression]->m_isConnected),Qt::black);
        }
        else
        {

            bool  tmp_TagQuality=true;
            float tmp_TagValue=0.0;
            QString tmp_alarmExpression;

            tmp_alarmExpression=alarmDescStruct.alarmExpression;

            foreach(alarm_expr_member_struct alarmExprMember, alarmDescStruct.vectAlarmExprMembers)
            {
                tmp_TagQuality&=hashCommonNodes[alarmExprMember.objectName]->m_isReaded;
            }

            if (tmp_TagQuality)
            {
                foreach(alarm_expr_member_struct alarmExprMember, alarmDescStruct.vectAlarmExprMembers)
                {
                    tmp_alarmExpression.replace(alarmExprMember.objectName+"["+QString::number(alarmExprMember.numInBuff)+"]",
                                                QString::number(hashCommonNodes[alarmExprMember.objectName]->m_srv.buff[alarmExprMember.numInBuff]));


                }
                //    logger->AddLog(alarmDescStruct.alarmExpression,Qt::black);
                tmp_TagValue=alarmScriptEngine.evaluate(tmp_alarmExpression).toNumber();

                if ( tmp_TagValue!=tmp_TagValue)  //тривиальная проверка на NaN
                {
                    tmp_TagValue=0.0;
                    tmp_TagQuality=false;
                    logger->AddLog("ERROR evaluate alarm formula: "+alarmDescStruct.alarmExpression, Qt::red);
                }
            }
            else
            {
                tmp_TagValue=0.0;
            }
            alarmDescStruct.alarmTag->SetValueQuality(tmp_TagValue,tmp_TagQuality);

        }
    }
}

//==================================================================================
