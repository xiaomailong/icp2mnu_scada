#include "mainwindow.h"

#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QNetworkInterface>
#include <QCryptographicHash>
#include <QRegExp>
#include <QtScript/QScriptEngine>
#include <QModelIndex>

#include "nodes.h"
#include "logger.h"
#include "alarm.h"
#include "odbcdb.h"
#include "configreader.h"
#include "nodedataviewer.h"




const char *trend_path="d:\\MNU_SCADA\\trends\\";   //каталог для трендовых файлов
const float min_float=-3.4028234663852886e+38;        //минимальное число float
float empty_file[17280];

//================================================================================================


QHash<QString, CommonNode *> hashCommonNodes;
QVector<CommonTrend *> vectCommonTrends;
QVector<alarm_tag_struct> vectAlarmTags;



//================================================================================================

//Подсистемы СКАДы

Logger *logger;
Alarms *alarms;
OdbcDb *alarmDB;

//===================================================================================================
bool MainWindow::CheckHash()
{


QString calculated_hash_result;
QString file_hash_result;

    //test - found network interfaces
    QList<QNetworkInterface> ni_list=QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface ni, ni_list)
    {
    //    logger->AddLog("Interface MAC: "+ni.hardwareAddress(),Qt::black);
    //    logger->AddLog("Interface name: "+ni.humanReadableName(),Qt::black);

        QList<QNetworkAddressEntry> nae_list=ni.addressEntries();
        foreach(QNetworkAddressEntry nae,nae_list)
        {
             //logger->AddLog("Interface  IP: "+nae.ip().toString(),Qt::black);

             if (nae.ip().toString().left(4)=="172.")
             {
                 QByteArray ba((nae.ip().toString()+ " " + ni.hardwareAddress()).toStdString().c_str());//,ni.hardwareAddress().toStdString().length());
                 QByteArray hash_result=QCryptographicHash::hash(ba,QCryptographicHash::Sha1);

                 calculated_hash_result=hash_result.toHex();


       //          logger->AddLog(QString("Interface  IP+MAC: ")+(nae.ip().toString()+ " " + ni.hardwareAddress()).toStdString().c_str(),Qt::black);
       //          logger->AddLog("Interface  MAC+IP hash: "+calculated_hash_result,Qt::black);
             }
        }


      //  logger->AddLog("----------------- ",Qt::black);

    }

    QFile file(qApp->applicationDirPath()+"\\main.hash");
    //QFile file("E:\\ALL_PROJECTS\\Qt5\\icp2mnu_scada_build\\release\\main.hash");
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    file_hash_result=file.readLine();

    //logger->AddLog(QString("File Hash: ")+file_hash_result,Qt::black);
    file.close();

    //end test code

    if (calculated_hash_result==file_hash_result) return true;



    return false;
}
//============================================================================================
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->buttonClose,SIGNAL(clicked()),this,SLOT(close()));

    connect(ui->testButton,SIGNAL(clicked()),this,SLOT(pushTestButton()));


    logger=Logger::Instance();
    logger->InstanceWindow();
    logger->AddLog("Старт системы",Qt::black);

    connect(ui->buttonMessagesShow,SIGNAL(clicked()),SLOT(buttonMessagesShow_clicked()));
    connect(ui->listWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(ViewNodeData(QListWidgetItem*)));


    alarms=new Alarms();


    for(int i=0;i<17280;i++)
    {
    empty_file[i]=min_float;
    }


      // configuration  - ConfigReader class
      //     To DO - add configuration from file to trends and alarms


    ConfigReader configReader;
    configReader.SetConfigFileName(qApp->applicationDirPath()+"\\mnu_scada.conf");

    if (configReader.OpenConfig())
    {
    //[NODES]
        QString objectName;
        QString objectType;
        QString IP_addr;
        uint port;
        uint port_repl;
        uint port_local;
        uint modbus_start_address;
        uint num_float_tags;

         while(configReader.ReadNextNode(objectName,objectType,IP_addr, port,
                                         port_repl,port_local,
                                         modbus_start_address, num_float_tags))
         {


             CommonNode *node = CommonNode::CreateNode(this, objectName,objectType,IP_addr, port,
                                                       port_repl,port_local,
                                                       modbus_start_address, num_float_tags);

             //char tabs[8];
             //if (node->m_nameObject.length()<9) sprintf(tabs,"\t\t");
             //else sprintf(tabs,"\t");


             char node_text[256];
             for (uint i=0;i<255;++i) node_text[i]=' ';
             node_text[255]=0;

            strcpy(&node_text[0],  node->m_nameObject.toStdString().c_str());
            node_text[strlen(node_text)]=' '; //уберем добавленный конец строки
            strcpy(&node_text[20], (node->m_IP_addr+":" + QString::number(node->m_port)).toStdString().c_str());
            node_text[strlen(node_text)]=' ';
            strcpy(&node_text[38], (QString("(") + node->m_typeObject).toStdString().c_str());
            node_text[strlen(node_text)]=' ';
            strcpy(&node_text[48], (QString(",tags=") + QString::number(node->m_srv.num_float_tags)+")").toStdString().c_str());
            node_text[strlen(node_text)]=' ';
            strcpy(&node_text[58], QString("connecting...").toStdString().c_str());
            node_text[strlen(node_text)]=' ';
            strcpy(&node_text[75], (QString(" ---> 127.0.0.1:")+ QString::number(node->m_port_local) + "(MADBUS)  === ").toStdString().c_str());
            node_text[strlen(node_text)]=' ';
            strcpy(&node_text[110], QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz").toStdString().c_str());

             ui->listWidget->addItem(node_text);


             //ui->listWidget->addItem(node->m_nameObject+":" + tabs + node->m_IP_addr+":" + QString::number(node->m_port)+
             //                         "(" + node->m_typeObject + ", tags=" + QString::number(node->m_srv.num_float_tags) + ")\t" +
             //                         "connecting..." + " ---> 127.0.0.1:" +
             //                         QString::number(node->m_port_local) + "(MADBUS) === " +
             //                         "   " + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));

             hashCommonNodes.insert(node->m_nameObject,node);

             //create Dir
             QDir nodeDir(QString(trend_path)+"\\"+node->m_nameObject);
             if (!nodeDir.exists()) nodeDir.mkdir(QString(trend_path)+"\\"+node->m_nameObject);

             logger->AddLog("Added Node: "+node->m_nameObject+": " + node->m_IP_addr+":" + QString::number(node->m_port)+
                                   "(" + node->m_typeObject + ", tags=" + QString::number(node->m_srv.num_float_tags),Qt::black);
          }

         //[TRENDS]
         //QString objectName; - redeclaration
         configReader.SeekStartConfig();
         QString trendName;
         uint numInBuff;

          while(configReader.ReadNextTrend(objectName,trendName,numInBuff))
          {
              if (hashCommonNodes.contains(objectName))
              {
                  CommonTrend *trend = new CommonTrend(objectName,trendName,numInBuff);
                  vectCommonTrends.append(trend);
                  logger->AddLog("Added Trend: "+trend->m_objectName+"  " + trend->m_trendName+"  " + QString::number(trend->m_numInBuff),Qt::darkBlue);
              }
              else
              {
                  logger->AddLog("ERROR Adding Trend (object not exists): "+objectName+"  " + trendName+"  " + QString::number(numInBuff),Qt::red);
              }
           }
          //[ALARMS]
         configReader.SeekStartConfig();

         QString alarmType;
         QString alarmExpression;
         QVector<alarm_expr_member_struct> alarmVectExprMembers;
         QString alarmComparison;
         float alarmValue;
         uint alarmDelay_s;
         QString alarmText;

         alarm_tag_struct alarmDescStruct;

         while(configReader.ReadNextAlarm(alarmType, alarmExpression, alarmVectExprMembers,
                                          alarmComparison, alarmValue, alarmDelay_s, alarmText))
         {


             alarmDescStruct.alarmType=alarmType;
             alarmDescStruct.alarmExpression=alarmExpression;
             alarmDescStruct.vectAlarmExprMembers=alarmVectExprMembers;

             if (alarmType=="connect")
             {
                 if (hashCommonNodes.contains(alarmExpression))
                 {
                    //connect alarms yellow("warning") color use
                    alarmDescStruct.alarmTag=new FloatTag();
                    alarms->AddAlarm("warning",OnValueChanged,alarmDescStruct.alarmTag,alarmComparison,alarmValue,alarmText, alarmDelay_s);
                    vectAlarmTags.append(alarmDescStruct);
                 }
                 else
                 {
                    logger->AddLog("ERROR Adding connect Alarm (object not exists): "+alarmExpression,Qt::red);
                 }
             }
             else  //information|warning|critical
             {
                  bool allMembersExist=true;
                  foreach(alarm_expr_member_struct alarmExprMember, alarmVectExprMembers)
                  {
                     if (!hashCommonNodes.contains(alarmExprMember.objectName)) allMembersExist=false;
                  }

                  if (allMembersExist)
                  {
                      alarmDescStruct.alarmTag=new FloatTag();
                      alarms->AddAlarm(alarmType,OnValueChanged,alarmDescStruct.alarmTag,alarmComparison,alarmValue,alarmText, alarmDelay_s);
                      vectAlarmTags.append(alarmDescStruct);
                  }
                  else
                  {
                      logger->AddLog("ERROR Adding Alarm (object not exists): "+alarmDescStruct.alarmType+"  "+alarmDescStruct.alarmExpression +
                                     " "+alarmText,Qt::red);

                  }

             //TEST
             //   foreach(alarm_expr_member_struct alarmExprMember, alarmVectExprMembers)
             //   {
             //       logger->AddLog("Added Alarm Exp Member: "+alarmDescStruct.alarmType+"  " + alarmDescStruct.alarmExpression + " " +
             //                      alarmExprMember.objectName+","+QString::number(alarmExprMember.numInBuff),Qt::black);
             //   }
             }


             //TEST
             //logger->AddLog("Added Alarm: "+alarmDescStruct.alarmType+"  " + alarmDescStruct.alarmExpression,Qt::black);
          }

         //[VIRTUAL_CONTROLLERS]
        configReader.SeekStartConfig();

        QString virtTagExpression;
        QVector<virt_expr_member_struct> virtTagVectExprMembers;
        virt_tag_struct virtTagDescStruct;

        while(configReader.ReadNextVirtualTag(objectName,numInBuff,virtTagExpression,virtTagVectExprMembers))
        {

            virtTagDescStruct.numInBuff=numInBuff;
            virtTagDescStruct.virtTagExpression=virtTagExpression;
            virtTagDescStruct.vectVirtTagExprMembers=virtTagVectExprMembers;

                 bool allMembersExist=true;
                 foreach(virt_expr_member_struct virtTagExprMember, virtTagVectExprMembers)
                 {
                    if (!hashCommonNodes.contains(virtTagExprMember.objectName)) allMembersExist=false;
                 }

                 if (allMembersExist)
                 {
                     VirtualNode *vn=0;
                     if (hashCommonNodes.contains(objectName))
                     vn=dynamic_cast<VirtualNode *>(hashCommonNodes[objectName]);
                     if (vn!=0)
                     {
                        vn->vectVirtTags.append(virtTagDescStruct);
                        logger->AddLog("Adding Virtual Tag Formula: "+objectName+"["+
                                       QString::number(virtTagDescStruct.numInBuff)+"] = "+virtTagDescStruct.virtTagExpression, Qt::black);
                     }
                     else
                     {
                         logger->AddLog("ERROR Adding Virtual Tag Formula(virt controller not exists): "+objectName+" "+
                                        QString::number(virtTagDescStruct.numInBuff)+" "+virtTagDescStruct.virtTagExpression, Qt::red);
                     }
                 }
                 else
                 {
                     logger->AddLog("ERROR Adding Virtual Tag Formula(object not exists in formula): "+objectName+" "+
                                    QString::number(virtTagDescStruct.numInBuff)+" "+virtTagDescStruct.virtTagExpression, Qt::red);

                 }

            //TEST
           //    foreach(virt_expr_member_struct virtTagExprMember, virtTagVectExprMembers)
           //    {
           //        logger->AddLog("Added Alarm Exp Member: "+virtTagDescStruct.virtTagExpression + " " +
           //                       virtTagExprMember.objectName+","+QString::number(virtTagExprMember.numInBuff),Qt::black);
           //    }



            //TEST
            //logger->AddLog("Added Alarm: "+alarmDescStruct.alarmType+"  " + alarmDescStruct.alarmExpression,Qt::black);
         }

        configReader.CloseConfig();
    }


    connect(this,SIGNAL(textSave2LogFile(int,QString,QString)),this,SLOT(TextSave2LogFile(int,QString,QString)));

    emit textSave2LogFile(-1, "","program started");


    connect(&timer5s_checkConnectAndSendToClients,SIGNAL(timeout()),this,SLOT(TimerEvent5s()));
    timer5s_checkConnectAndSendToClients.start(5000);
    trendWriterThread.start();

     alarmDB=new OdbcDb("fire_alarmdb","SYSDBA","784523");
     alarmDB->AddAlarm2DB("Alarms Server Started...");

     connect(&timer1s_setAlarmTags,SIGNAL(timeout()),this,SLOT(TimerEvent1s_setAlarmsTags()));
     timer1s_setAlarmTags.start(1000);

     connect(ui->button_Confirm5Alarms, SIGNAL(clicked()),alarms,SLOT(Kvitirovat2()));
     connect(alarms,SIGNAL(EnabledAlarmsChanged(QList<Alarm*>*,bool)),SLOT(alarmsChanged(QList<Alarm*>*,bool)));
}
//=========================================================================================================
void MainWindow::alarmsChanged(QList < Alarm* > *pEnabledAlarmList, bool onlyColorChange)
{

    int i=0;

    if (!onlyColorChange)
    {
    ui->alarmWidget->clear();
        foreach(Alarm *alarm, *pEnabledAlarmList)
        {

            //if (i<5)
            {
                QString alarmstr;

                alarmstr.append(alarm->GetDateTimeOfAlarmActivating());
                alarmstr.append(" ");
                alarmstr.append(alarm->GetTextOfAlarm());
                while(alarmstr.length()<92) alarmstr.append(" ");

                alarmstr.append(QString::number(alarm->alarmValue,'f',2));

                while(alarmstr.length()<100) alarmstr.append(" ");

                if (alarm->IsActive()) alarmstr.append("АКТ");
                else alarmstr.append(QString("НЕАКТ ") + alarm->GetDateTimeOfAlarmDeactivating());
                while(alarmstr.length()<128) alarmstr.append(" ");

                if (!alarm->IsConfirmed()) alarmstr.append("НЕКВИТ");
                else alarmstr.append(QString("КВИТ ")+alarm->GetDateTimeOfAlarmConfirming());


                ui->alarmWidget->addItem(alarmstr);

                if (alarm->IsActive())
                {
                    ui->alarmWidget->item(i)->setBackground(alarm->actColor);
                }
                else
                {
                    ui->alarmWidget->item(i)->setBackground(alarm->nonactColor);
                }
            }
        ++i;
        }
    }
    else
    {
        foreach(Alarm *alarm, *pEnabledAlarmList)
        {

            //if (i<5)
            {
                if (!alarm->IsConfirmed() && alarm->IsActive())
                {
                    if (ui->alarmWidget->item(i)->background()==alarm->actColor)
                    {
                        ui->alarmWidget->item(i)->setBackground(alarm->act2Color);
                    }
                    else
                    {
                        ui->alarmWidget->item(i)->setBackground(alarm->actColor);
                    }
                }
//                else
//                {
//                    ui->alarmWidget->item(i)->setBackground(alarm->actColor);
//                }
            }
        ++i;
        }

    }
}
//=========================================================================================================
void MainWindow::buttonMessagesShow_clicked()
{
    logger->InstanceWindow()->setGeometry(this->width()-logger->InstanceWindow()->width(),
                                //messagesWindow->geometry().y()- messagesWindow->pos().y(),
                                40,
                                logger->InstanceWindow()->width(),
                                logger->InstanceWindow()->height());

    logger->InstanceWindow()->setWindowFlags(Qt::WindowStaysOnTopHint);
    logger->InstanceWindow()->show();

}


//=========================================================================================================
void MainWindow::TimerEvent1s_setAlarmsTags()
{

/*    static int firstcycles=0;
    //пропустим 2 цикла - дадим возможность подключиться
    if (firstcycles<2)
    {
        firstcycles++;
        return;
    }
*/
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

            foreach(alarm_expr_member_struct alarmExprMember, alarmDescStruct.vectAlarmExprMembers)
            {
                  tmp_TagQuality&=hashCommonNodes[alarmExprMember.objectName]->m_isReaded;
            }

            if (tmp_TagQuality)
            {
                foreach(alarm_expr_member_struct alarmExprMember, alarmDescStruct.vectAlarmExprMembers)
                {
                    alarmDescStruct.alarmExpression.replace(alarmExprMember.objectName+"["+QString::number(alarmExprMember.numInBuff)+"]",
                                                            QString::number(hashCommonNodes[alarmExprMember.objectName]->m_srv.buff[alarmExprMember.numInBuff]));


                }
            //    logger->AddLog(alarmDescStruct.alarmExpression,Qt::black);
                tmp_TagValue=alarmScriptEngine.evaluate(alarmDescStruct.alarmExpression).toNumber();

                if ( tmp_TagValue!=tmp_TagValue)  //тривиальная проверка на NaN
                {
                   tmp_TagValue=0.0;
                   tmp_TagQuality=false;
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
//=========================================================================================================
void MainWindow::TimerEvent5s()
{

    //for (int i=0;i<listCommonNodes.size();++i)
    foreach(CommonNode* node,hashCommonNodes)
    {
       if (node->m_isReaded &&
           !node->m_srv.m_pServerSocket->isListening())
       {
           node->m_srv.m_pServerSocket->listen(QHostAddress::Any, node->m_port_local);

       }

       if( node->m_isReaded &&
           node->m_srv.m_pServerSocket->isListening())
       {
           for(int j=0;j<node->m_srv.m_pClientSocketList.size();j++)
           {
               node->m_srv.m_pClientSocketList.at(j)->write((char *)node->m_srv.buff,node->m_srv.num_float_tags*4);

           }


       }


       if( !node->m_isReaded &&
           node->m_srv.m_pServerSocket->isListening())
       {

           for(int j=0;j<node->m_srv.m_pClientSocketList.size();j++)
           {
               node->m_srv.m_pClientSocketList.at(j)->close();
           }

           node->m_srv.m_pServerSocket->close();
           node->m_srv.m_pClientSocketList.clear();

       }


    }


}

//==================================================================================

MainWindow::~MainWindow()
{

    emit textSave2LogFile(-1, "","program closed");

    timer5s_checkConnectAndSendToClients.stop();
    timer1s_setAlarmTags.stop();

    //for (int i=0;i<listCommonNodes.size();++i)
    foreach(CommonNode* node,hashCommonNodes)
    {
        delete node;
    }

    //Sleep(3000); //delay to get time to threads stop

    foreach(alarm_tag_struct alarmTg, vectAlarmTags)
    {
        delete alarmTg.alarmTag;
    }

    foreach(Alarm* al,alarms->allAlarmsList)
    {
        delete al;
    }

    delete alarms;

    foreach(CommonTrend* trend,vectCommonTrends)
    {
        delete trend;
    }

    //hashCommonNodes.clear();
    alarmDB->AddAlarm2DB("Alarms Server Closed...");
    delete alarmDB;

    delete ui;

}
//===================================================================================
void MainWindow::TextChanged(int iUzel, QString objectName, QString newText)
{

//char tabs[8];

hashCommonNodes[objectName]->m_text_client=newText;

char node_text[256];
for (uint i=0;i<255;++i) node_text[i]=' ';
node_text[255]=0;

strcpy(&node_text[0],  objectName.toStdString().c_str());
node_text[strlen(node_text)]=' '; //уберем добавленный конец строки
strcpy(&node_text[20], (hashCommonNodes[objectName]->m_IP_addr+":" + QString::number(hashCommonNodes[objectName]->m_port)).toStdString().c_str());
node_text[strlen(node_text)]=' ';
strcpy(&node_text[38], (QString("(") + hashCommonNodes[objectName]->m_typeObject).toStdString().c_str());
node_text[strlen(node_text)]=' ';
strcpy(&node_text[48], (QString(",tags=") + QString::number(hashCommonNodes[objectName]->m_srv.num_float_tags)+")").toStdString().c_str());
node_text[strlen(node_text)]=' ';
strcpy(&node_text[58], (hashCommonNodes[objectName]->m_text_client+ " " + hashCommonNodes[objectName]->m_text_repl).toStdString().c_str());
node_text[strlen(node_text)]=' ';
strcpy(&node_text[75], (QString(" ---> 127.0.0.1:")+ QString::number(hashCommonNodes[objectName]->m_port_local) + "(MADBUS)  === ").toStdString().c_str());
node_text[strlen(node_text)]=' ';
strcpy(&node_text[110], QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz").toStdString().c_str());

//if (objectName.length()<9) sprintf(tabs,"\t\t");
//else sprintf(tabs,"\t");


// ui->listWidget->item(iUzel)->setText( objectName+":" + tabs + hashCommonNodes[objectName]->m_IP_addr+":" + QString::number(hashCommonNodes[objectName]->m_port)+
//                                       "(" + hashCommonNodes[objectName]->m_typeObject + ", tags=" + QString::number(hashCommonNodes[objectName]->m_srv.num_float_tags) + ")\t" +
//                                       hashCommonNodes[objectName]->m_text_client+ " " + hashCommonNodes[objectName]->m_text_repl + " ---> 127.0.0.1:" +
//                                       QString::number(hashCommonNodes[objectName]->m_port_local) + "(MADBUS) === " +
//                                       "   " + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));

ui->listWidget->item(iUzel)->setText(node_text);

 logger->AddLog("Network: " + hashCommonNodes[objectName]->m_nameObject+":" + hashCommonNodes[objectName]->m_IP_addr+":" +
                QString::number(hashCommonNodes[objectName]->m_port)+
                "(" + hashCommonNodes[objectName]->m_typeObject  + ") === " + hashCommonNodes[objectName]->m_text_client,Qt::darkGreen);

}
//========================================================================================
void MainWindow::TextChanged_repl(int iUzel, QString objectName, QString newText)
{

    hashCommonNodes[objectName]->m_text_repl=newText;

    char node_text[256];
    for (uint i=0;i<255;++i) node_text[i]=' ';
    node_text[255]=0;

    strcpy(&node_text[0],  objectName.toStdString().c_str());
    node_text[strlen(node_text)]=' '; //уберем добавленный конец строки
    strcpy(&node_text[20], (hashCommonNodes[objectName]->m_IP_addr+":" + QString::number(hashCommonNodes[objectName]->m_port)).toStdString().c_str());
    node_text[strlen(node_text)]=' ';
    strcpy(&node_text[38], (QString("(") + hashCommonNodes[objectName]->m_typeObject).toStdString().c_str());
    node_text[strlen(node_text)]=' ';
    strcpy(&node_text[48], (QString(",tags=") + QString::number(hashCommonNodes[objectName]->m_srv.num_float_tags)+")").toStdString().c_str());
    node_text[strlen(node_text)]=' ';
    strcpy(&node_text[58], (hashCommonNodes[objectName]->m_text_client+ " " + hashCommonNodes[objectName]->m_text_repl).toStdString().c_str());
    node_text[strlen(node_text)]=' ';
    strcpy(&node_text[75], (QString(" ---> 127.0.0.1:")+ QString::number(hashCommonNodes[objectName]->m_port_local) + "(MADBUS)  === ").toStdString().c_str());
    node_text[strlen(node_text)]=' ';
    strcpy(&node_text[110], QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz").toStdString().c_str());



    ui->listWidget->item(iUzel)->setText(node_text);

 //ui->listWidget->item(iUzel)->setText( objectName+":" + tabs + hashCommonNodes[objectName]->m_IP_addr+":" + QString::number(hashCommonNodes[objectName]->m_port)+
 //                                      "(" + hashCommonNodes[objectName]->m_typeObject + ", tags=" + QString::number(hashCommonNodes[objectName]->m_srv.num_float_tags) + ")\t" +
 //                                      hashCommonNodes[objectName]->m_text_client+ " " + hashCommonNodes[objectName]->m_text_repl + " ---> 127.0.0.1:" +
 //                                      QString::number(hashCommonNodes[objectName]->m_port_local) + "(MADBUS) === " +
  //                                     "   " + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
}
//========================================================================================

void MainWindow::TextSave2LogFile(int iUzel, QString objectName, QString newText)
{


    if (iUzel==-1)    //global event
    {
        QString filename;

        filename.sprintf("netlog\\%s.log",QDateTime::currentDateTime().toString("yyyy_MM_dd").toStdString().c_str());


        QFile netlog(filename);



            if (netlog.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
                {
                QString tmp;
                 tmp.sprintf("%s --- %s\n",QDateTime::currentDateTime().toString("hh:mm:ss.zzz").toStdString().c_str(),
                             newText.toStdString().c_str());
                netlog.write( tmp.toStdString().c_str());
                netlog.close();
                }
    }
    else              //uzel event
    {
       // QMessageBox::information(this,"Configuration message","Your configuration is incorrect - no IP address!!!",QMessageBox::Ok);
        QString filename;

        filename.sprintf("netlog\\%s.log",QDateTime::currentDateTime().toString("yyyy_MM_dd").toStdString().c_str());


        QFile netlog(filename);



            if (netlog.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
                {
                QString tmp;
                 tmp.sprintf("%s --- %s(%s) %s\n",QDateTime::currentDateTime().toString("hh:mm:ss.zzz").toStdString().c_str(),
                             hashCommonNodes[objectName]->m_nameObject.toStdString().c_str(),
                             hashCommonNodes[objectName]->m_IP_addr.toStdString().c_str(),
                             newText.toStdString().c_str());
                netlog.write( tmp.toStdString().c_str());
                netlog.close();
                }

    }




}
//========================================================================================
void FuncFileWriter(CommonTrend *this_trend_info, char *str_date, uint time_pos)
{
//char filename[256];

QString filename;

filename.sprintf("%s%s\\%s_%s.trn",trend_path,this_trend_info->m_objectName.toStdString().c_str(),this_trend_info->m_trendName.toStdString().c_str(),str_date);

QFile trend(filename);


if (!trend.exists())  //Open(filename,CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyNone,NULL))
    {

        if (trend.open(QIODevice::ReadWrite))
        {
            trend.write((char *)&empty_file[0],69120);
            trend.seek(time_pos*4);//, CFile::begin);
            trend.write((char *)&(hashCommonNodes[this_trend_info->m_objectName]->m_srv.buff[this_trend_info->m_numInBuff]),4);
            trend.close();
        }
    }
    else
    {
        trend.open(QIODevice::ReadWrite);
        trend.seek(time_pos*4);//, CFile::begin);
        trend.write((char *)&(hashCommonNodes[this_trend_info->m_objectName]->m_srv.buff[this_trend_info->m_numInBuff]),4);
        trend.close();
    }

return;
}
//=========================================================================================

void TrendWriterThread::run()
{

int itime,iprevtime=100;

    for(;;)
    {
        Sleep(200);

        QDateTime currDateTime=QDateTime::currentDateTime();

        itime=currDateTime.time().second() / 5;
        if (itime!=iprevtime)
        {
            char buf_date[20];
            int time22=(currDateTime.time().hour()*60*60+currDateTime.time().minute()*60+currDateTime.time().second())/5;
            sprintf(buf_date,"%.2u_%.2u_%.4u",currDateTime.date().day(),currDateTime.date().month(),currDateTime.date().year());

            foreach (CommonTrend *tr,vectCommonTrends)
            {

                if (hashCommonNodes[tr->m_objectName]->m_isReaded)
                {
                    FuncFileWriter(tr,buf_date,time22);
                }
            }
        }
        iprevtime=itime;
        if (CheckThreadStop()) return;
    }

}
//=========================================================================================
void MainWindow::pushTestButton()
{
    static int first_call=true;

    static QScriptEngine engine;
    if (first_call)
    {
        QString fileName = qApp->applicationDirPath()+"\\mnu_scada.js";
        QFile scriptFile(fileName);
        scriptFile.open(QIODevice::ReadOnly);
        QTextStream stream(&scriptFile);
        QString contents = stream.readAll();
        scriptFile.close();

        engine.evaluate(contents, fileName);

        //QString my_fun="function my_fun( x ) { return x*x; } function my_fun2(x) {return x*x*x }";
        //engine.evaluate(my_fun);
    }
    else
    {
        float res=engine.evaluate("my_fun2(5)+my_fun(2)").toNumber();
        ui->testButton->setText(QString::number(res));
    }
    first_call=false;


}

//=========================================================================================
void MainWindow::ViewNodeData(QListWidgetItem *item)
{
/*
    foreach(CommonNode *node,hashCommonNodes)
    {
        if (node->m_this_number==ui->listWidget->
        {
            NodeDataViewer *ndw=new NodeDataViewer(node ,this);
            ndw->show();
            break;
        }
    }
*/

    QString objName=item->text();
        qDebug() << objName;
    objName=objName.left(objName.indexOf(' '));
        qDebug() << objName;

    if (hashCommonNodes.contains(objName))
    {
        NodeDataViewer *ndw=new NodeDataViewer(hashCommonNodes[objName] ,this);
        ndw->show();
    }
}
//=========================================================================================
