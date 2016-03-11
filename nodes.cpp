#include "nodes.h"


//allocate memory for static member
uint CommonNode::nodes_counter=0;


//сравнение для сортировки по убыванию длины имени объекта, лямбду не принимает
bool operator<(const virt_expr_member_struct &a, const virt_expr_member_struct &b)
{
    return a.objectName.length() > b.objectName.length();
}
//================================================================================
CommonNode::CommonNode()
{
    memset(m_srv.buff,0.0,100);
    m_text_client="connecting...";
    m_text_repl="";
    m_isConnected=false;
    m_isReaded=false;
    ss = ScadaServer::GetInstance();
}
//================================================================================
CommonNode::~CommonNode()
{

}
//================================================================================
CommonNode* CommonNode::CreateNode(QString objectName,QString objectType,
                                   QString IP_addr, uint port,
                                   uint port_repl,uint port_local,
                                   uint modbus_start_address,
                                   uint num_float_tags)
{

CommonNode *node=NULL;

    if (objectType=="modbus")
    {
        node = new ModbusNode(nodes_counter,objectName,objectType,
                                          IP_addr, port,port_repl,port_local,
                                          modbus_start_address,num_float_tags);
    }

    if (objectType=="mnu_scada")
    {
        node = new MnuScadaNode(nodes_counter,objectName,objectType,
                                            IP_addr, port,port_repl,port_local,
                                            modbus_start_address,num_float_tags);

    }

    if (objectType=="virtual")
    {
        node = new VirtualNode(nodes_counter,objectName,objectType,
                                          IP_addr, port,port_repl,port_local,
                                          modbus_start_address,num_float_tags);

    }

    nodes_counter++;
    return node;
}
//================================================================================
QString CommonNode::FormattedNodeString()
{

    char node_text[256];
    for (uint i=0;i<255;++i) node_text[i]=' ';
    node_text[255]=0;

    //Небезопасно - по хорошему переделать на strncpy, а лучше на strlcpy, но его нет в mingw

    strcpy(&node_text[0],  m_nameObject.toStdString().c_str());
    node_text[strlen(node_text)]=' '; //уберем добавленный конец строки
    strcpy(&node_text[20], (m_IP_addr+":" + QString::number(m_port)).toStdString().c_str());
    node_text[strlen(node_text)]=' ';
    strcpy(&node_text[39], (QString("(") + m_typeObject).toStdString().c_str());
    node_text[strlen(node_text)]=' ';
    strcpy(&node_text[49], (QString(",tags=") + QString::number(m_srv.num_float_tags)+")").toStdString().c_str());
    node_text[strlen(node_text)]=' ';
    strcpy(&node_text[59], (m_text_client+ " " + m_text_repl).toStdString().c_str());
    node_text[strlen(node_text)]=' ';
    strcpy(&node_text[76], (QString(" ---> 127.0.0.1:")+ QString::number(m_port_local) + "(MADBUS)  === ").toStdString().c_str());
    node_text[strlen(node_text)]=' ';
    strcpy(&node_text[111], QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz").toStdString().c_str());

   return QString(node_text);

/*  c++ + Qt
    QString node_text;
    node_text.fill(' ',134);
    node_text.replace(0,m_nameObject.length(),m_nameObject);
    node_text.replace(20,(m_IP_addr+":" + QString::number(m_port)).length(),(m_IP_addr+":" + QString::number(m_port)));
    node_text.replace(39,(QString("(") + m_typeObject).length(),(QString("(") + m_typeObject));
    node_text.replace(49,(QString(",tags=") + QString::number(m_srv.num_float_tags)+")").length(),(QString(",tags=") + QString::number(m_srv.num_float_tags)+")"));
    node_text.replace(59,(m_text_client+ " " + m_text_repl).length(),(m_text_client+ " " + m_text_repl));
    node_text.replace(76,(QString(" ---> 127.0.0.1:")+ QString::number(m_port_local) + "(MADBUS)  === ").length(),(QString(" ---> 127.0.0.1:")+ QString::number(m_port_local) + "(MADBUS)  === "));
    node_text.replace(111,QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz").length(),QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
    return node_text;
*/
}
//================================================================================
ModbusNode::ModbusNode(int this_number,QString objectName,QString objectType,
                          QString IP_addr, uint port,
                          uint port_repl,uint port_local,
                          uint modbus_start_address,
                          uint num_float_tags)
{
    m_this_number=this_number;

    m_nameObject=objectName;
    m_typeObject=objectType;
    m_IP_addr=IP_addr;
    m_port=port;
    m_port_repl=port_repl;
    m_port_local=port_local;
    m_modbus_start_address=modbus_start_address;
    m_srv.num_float_tags=num_float_tags;

    start();
}
//================================================================================
void ModbusNode::run()
{
int res;

modbus_t *mb;
uint16_t tab_reg[200];

            if (this->m_IP_addr.isEmpty())
            {
                //QMessageBox::information(this,"Configuration message","Your configuration is incorrect - no IP address!!!",QMessageBox::Ok);
                emit textchange(this->m_this_number, this->m_nameObject, "ERROR-No IP adress!!!");
                return;
            }

            //inialized context
            mb = modbus_new_tcp(this->m_IP_addr.toStdString().c_str(), this->m_port);

      for(;;)
      {
            //connect if not connected
            if(!m_isConnected)
            {

                res=modbus_connect(mb);

                if (res!=-1)
                {
                   m_isConnected=true;
                   modbus_set_slave(mb, 1);

                   emit textchange(this->m_this_number,this->m_nameObject,  "connected");
                   emit textSave2LogFile(this->m_this_number,this->m_nameObject,  "connected");
                   //srv->m_pServerSocket->setSocketOption(QAbstractSocket:: KeepAliveOption, 1);
                }
                else
                {
                    m_isConnected=false;
                    //emit textchange(this->m_this_number,this->m_nameObject,  "connecting failure");
                }

            }

            if (m_isConnected)
            {
                res=modbus_read_registers(mb, this->m_modbus_start_address, m_srv.num_float_tags*2, tab_reg);
                if (res==m_srv.num_float_tags*2)
                {
                    for (uint nn=0;nn < m_srv.num_float_tags;++nn)
                    {
                        m_srv.buff[nn]=modbus_get_float(&tab_reg[nn*2]);


                    }
                    m_isReaded=true;
                }

                else  //error read? closing
                {
                    m_isConnected=false;
                    m_isReaded=false;
                    modbus_close(mb);
                    emit textchange(this->m_this_number,this->m_nameObject,  "disconnected");
                    emit textSave2LogFile(this->m_this_number,this->m_nameObject,  "disconnected");
                }


            }


        for(int i=0;i<22;++i) //4.4 sec delay
        {
        if (CheckThreadStop()) return;
        Sleep(200);
        }

    }//for(;;)


}
//=============================================================================
MnuScadaNode::MnuScadaNode(int this_number, QString objectName, QString objectType, QString IP_addr, uint port, uint port_repl, uint port_local, uint modbus_start_address, uint num_float_tags)
{
    socket = new QTcpSocket(this);

    m_this_number=this_number;

    m_nameObject=objectName;
    m_typeObject=objectType;
    m_IP_addr=IP_addr;
    m_port=port;
    m_port_repl=port_repl;
    m_port_local=port_local;
    m_modbus_start_address=modbus_start_address;
    m_srv.num_float_tags=num_float_tags;
    m_num_reads=0;
    m_sec_counter=1;

    connect(socket, SIGNAL(connected()),this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()),this, SLOT(disconnected()));
    connect(socket, SIGNAL(bytesWritten(qint64)),this, SLOT(bytesWritten(qint64)));
    connect(socket, SIGNAL(readyRead()),this, SLOT(readyRead()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error(QAbstractSocket::SocketError)));
    connect(&timerConnectLoop,SIGNAL(timeout()),this,SLOT(TimerConnectLoopEvent()));

    need_restart_repl=false;

    timerConnectLoop.start(1000);
}
//================================================================================
MnuScadaNode::~MnuScadaNode()
{
    /*
    if (isConnected==true)
    {
        socket->disconnectFromHost();
        socket->waitForDisconnected(5000);
    }
    */

    timerConnectLoop.stop(); // - error QObject::killTimers: timers cannot be stopped from another thread
    delete socket;

}
//===================================================================================
void MnuScadaNode::doConnect()
{
    // this is not blocking call
    socket->connectToHost(m_IP_addr, m_port);
    //isConnecting=true;
    // we need to wait...
    //if(!socket->waitForConnected(5000))
    //{
    //    qDebug() << "Error: " << socket->errorString();
    //}
}
//==================================================================================
void MnuScadaNode::TimerConnectLoopEvent()
{
//    qDebug() << "timer connect loop event: ";

    if (getStateConnected()==false && getStateConnecting()==false)
    {
        doConnect();

    }

    //if no data 20 seconds - reconnect

    if (m_sec_counter==0) //раз в 20 секунд
    {
        if (m_num_reads==0 && getStateConnected()==true)
        {
            socket->disconnectFromHost();
            //qDebug() << "error - no data in 20 sec";
        }
        m_num_reads=0;
    }

    m_sec_counter=(m_sec_counter+1)%20;

}
//====================================================================================
void MnuScadaNode::connected()
{
    qDebug() << "connected...";

    m_isReaded=false;
    m_isConnected=true;
    emit textchange(this->m_this_number,this->m_nameObject,  "connected");
    emit textSave2LogFile(this->m_this_number,this->m_nameObject,  "connected");

    if (this->isRunning()==false)
    {
        start();
        emit textchange_repl(this->m_this_number,this->m_nameObject,  "tr.repl");
    }
    else
    {
        need_restart_repl=true;

    }

}
//=====================================================================================
void MnuScadaNode::error(QAbstractSocket::SocketError err)
{
    socket->disconnectFromHost();
    qDebug() << "error " + QString::number(err);
    m_isReaded=false;
}
//=======================================================================================
void MnuScadaNode::disconnected()
{
    m_isReaded=false;
    m_isConnected=false;

    emit textchange(this->m_this_number,this->m_nameObject,  "disconnected");
    emit textSave2LogFile(this->m_this_number,this->m_nameObject,  "disconnected");

    qDebug() << "disconnected...";
}
//=======================================================================================
void MnuScadaNode::bytesWritten(qint64 bytes)
{
    qDebug() << bytes << " bytes written...";
}
//=======================================================================================
void MnuScadaNode::readyRead()
{
    //qDebug() << "reading..."+QString::number(socket->bytesAvailable());


    // read the data from the socket
    if (socket->bytesAvailable()==m_srv.num_float_tags*4) // || socket->bytesAvailable()==number_float_tags*4+4) //ribalci gluk
    {
        socket->read((char *)(m_srv.buff),socket->bytesAvailable());
        //qDebug() << m_srv.buff[0] << m_srv.buff[1] << m_srv.buff[2] << m_srv.buff[3];
        m_isReaded=true;
        m_num_reads++;
    }
    else
    {
        socket->disconnectFromHost();
        m_isReaded=false;
    }
}
//===========================================================================================
/*
QAbstractSocket::UnconnectedState	0	The socket is not connected.
QAbstractSocket::HostLookupState	1	The socket is performing a host name lookup.
QAbstractSocket::ConnectingState	2	The socket has started establishing a connection.
QAbstractSocket::ConnectedState     3	A connection is established.
QAbstractSocket::BoundState         4	The socket is bound to an address and port.
QAbstractSocket::ClosingState       6	The socket is about to close (data may still be waiting to be written).
QAbstractSocket::ListeningState     5	For internal use only.
*/
//=============================================================
bool MnuScadaNode::getStateConnected()
{
   return socket->state()==QAbstractSocket::ConnectedState;
}
//=============================================================
bool MnuScadaNode::getStateConnecting()
{
   return socket->state()==QAbstractSocket::HostLookupState || socket->state()==QAbstractSocket::ConnectingState;
}
//=============================================================
//поток восстановления трендовых файлов после отключения/пропадания связи с узлов "mnu_scada"
//+функции из проекта на VC++ 2003 - переделано под куте
// tags replication  ===========================================================================================

int mod(int number1,int divider)
{
div_t fff=div(number1, divider);
return fff.rem;
}
//=======================================================================================
void PlusOneDay(int &iYear,int &iMonth,int &iDay)
{
    if ((iMonth==12) && (iDay==31)) {iYear++;iMonth=1;iDay=1;return;}

    if ((iMonth==1) && (iDay==31)) {iMonth=2;iDay=1;return;}

if ((mod(iYear,4)==0) && (iMonth==2) && (iDay==29)) {iMonth=3;iDay=1;return;}
if ((mod(iYear,4)!=0) && (iMonth==2) && (iDay==28)) {iMonth=3;iDay=1;return;}

if ((iMonth==3) && (iDay==31)) {iMonth=4;iDay=1;return;}
if ((iMonth==4) && (iDay==30)) {iMonth=5;iDay=1;return;}
if ((iMonth==5) && (iDay==31)) {iMonth=6;iDay=1;return;}
if ((iMonth==6) && (iDay==30)) {iMonth=7;iDay=1;return;}
if ((iMonth==7) && (iDay==31)) {iMonth=8;iDay=1;return;}
if ((iMonth==8) && (iDay==31)) {iMonth=9;iDay=1;return;}
if ((iMonth==9) && (iDay==30)) {iMonth=10;iDay=1;return;}
if ((iMonth==10) && (iDay==31)) {iMonth=11;iDay=1;return;}
if ((iMonth==11) && (iDay==30)) {iMonth=12;iDay=1;return;}
iDay++;return;
}
//========================================================================================
void MinusOneDay(int &iYear,int &iMonth,int &iDay)
{
    if ((iMonth==1) && (iDay==1)) {iYear--;iMonth=12;iDay=31;return;}

    if ((iMonth==2) && (iDay==1)) {iMonth=1;iDay=31;return;}

if ((mod(iYear,4)==0) && (iMonth==3) && (iDay==1)) {iMonth=2;iDay=29;return;}
if ((mod(iYear,4)!=0) && (iMonth==3) && (iDay==1)) {iMonth=2;iDay=28;return;}

if ((iMonth==4) && (iDay==1)) {iMonth=3;iDay=31;return;}
if ((iMonth==5) && (iDay==1)) {iMonth=4;iDay=30;return;}
if ((iMonth==6) && (iDay==1)) {iMonth=5;iDay=31;return;}
if ((iMonth==7) && (iDay==1)) {iMonth=6;iDay=30;return;}
if ((iMonth==8) && (iDay==1)) {iMonth=7;iDay=31;return;}
if ((iMonth==9) && (iDay==1)) {iMonth=8;iDay=31;return;}
if ((iMonth==10) && (iDay==1)) {iMonth=9;iDay=30;return;}
if ((iMonth==11) && (iDay==1)) {iMonth=10;iDay=31;return;}
if ((iMonth==12) && (iDay==1)) {iMonth=11;iDay=30;return;}
iDay--;return;
}
//=================================================================================
void MinusInterval(int &iYear,int &iMonth,int &iDay, int &iHour,int &iMinute,int &iSecond, int interval_sec)
{
    if (interval_sec>60*60*24)
    {
        MinusOneDay(iYear,iMonth,iDay);
        MinusInterval(iYear,iMonth,iDay, iHour,iMinute,iSecond,interval_sec-60*60*24);
    }
    else
    {
        if (interval_sec<=(iHour*60*60+iMinute*60+iSecond))
         {
             int temp=(iHour*60*60+iMinute*60+iSecond)-interval_sec;
             iHour=temp/3600;
             iMinute=(temp-iHour*3600)/60;
             iSecond=temp-iHour*3600-iMinute*60;
         }
         else
         {
             MinusOneDay(iYear,iMonth,iDay);
             int temp=24*60*60+(iHour*60*60+iMinute*60+iSecond)-interval_sec;

             iHour=temp/3600;
             iMinute=(temp-iHour*3600)/60;
             iSecond=temp-iHour*3600-iMinute*60;
         }
    }
}
//=============================================================================================================
void PlusInterval(int &iYear,int &iMonth,int &iDay, int &iHour,int &iMinute,int &iSecond, int interval_sec)
{
    if (interval_sec>60*60*24)
    {
        PlusOneDay(iYear,iMonth,iDay);
        PlusInterval(iYear,iMonth,iDay, iHour,iMinute,iSecond,interval_sec-60*60*24);
    }
    else
    {
        if ((interval_sec+iHour*60*60+iMinute*60+iSecond)<24*60*60)
        {
             int temp=iHour*60*60+iMinute*60+iSecond+interval_sec;
             iHour=temp/3600;
             iMinute=(temp-iHour*3600)/60;
             iSecond=temp-iHour*3600-iMinute*60;
        }
        else
        {
             PlusOneDay(iYear,iMonth,iDay);
             int temp=-24*60*60+(iHour*60*60+iMinute*60+iSecond)+interval_sec;

             iHour=temp/3600;
             iMinute=(temp-iHour*3600)/60;
             iSecond=temp-iHour*3600-iMinute*60;
        }
    }
}
//================================================================================================
int Interval(int sYear,int sMonth,int sDay, int sHour,int sMinute,int sSecond,int eYear,int eMonth,int eDay, int eHour,int eMinute,int eSecond)
{
//if ((sYear>eYear) || ((sYear==eYear) && (sMonth>eMonth)) ||
//	((sYear==eYear) && (sMonth==eMonth) && (sDay>eDay)) return 0;
int summa_sec=0;
//если день старт и енд не равны, то срабатывает один из циклов
//startday < end
for (;((sYear<eYear) || ((sYear==eYear) && (sMonth<eMonth)) || ((sYear==eYear) && (sMonth==eMonth) && (sDay<eDay)));PlusOneDay(sYear,sMonth,sDay))
summa_sec+=86400;
//startday > end  - пока закоментировал - не проверено
//for (;((sYear>eYear) || ((sYear==eYear) && (sMonth>eMonth)) || ((sYear==eYear) && (sMonth==eMonth) && (sDay>eDay)));MinusOneDay(sYear,sMonth,sDay))
//summa_sec-=86400;
//startday == end
summa_sec+=(eHour-sHour)*3600+(eMinute-sMinute)*60+eSecond-sSecond;
return summa_sec;
}
//=================================================================================================
void MnuScadaNode::run()
{
    QTcpSocket repl_socket;
    int aligner=0;

    msleep(1000);

        for (int cycles_cnt=0;cycles_cnt<2;++cycles_cnt)
        {
            foreach(CommonTrend *trend, ss->vectCommonTrends)
            {

                if (this->m_nameObject==trend->m_objectName)
                {
                        repl_socket.connectToHost(m_IP_addr, m_port_repl);
                        if (repl_socket.waitForConnected(5000))  //true if connected
                        {

                            //qDebug() << "repl connected";
                            float buff_file[17280];

                            QDateTime now_time=QDateTime::currentDateTime();

                            int rrYear_st=now_time.date().year();
                            int rrMonth_st=now_time.date().month();
                            int rrDay_st=now_time.date().day();
                            int rrHour_st=0;
                            int rrMinute_st=0;
                            int rrSecond_st=0;

                            int rrYear_end=now_time.date().year();
                            int rrMonth_end=now_time.date().month();
                            int rrDay_end=now_time.date().day();
                            PlusOneDay(rrYear_end,rrMonth_end,rrDay_end);

                            int Position_end=(now_time.time().hour()*60*60+now_time.time().minute()*60+now_time.time().second())/5;

                            //interval of trend replication - 4 weeks
                            MinusInterval(rrYear_st,rrMonth_st,rrDay_st,rrHour_st,rrMinute_st,rrSecond_st,86400*7*4);

                            for(;!(rrYear_st==rrYear_end && rrMonth_st==rrMonth_end && rrDay_st==rrDay_end);
                            PlusOneDay(rrYear_st,rrMonth_st,rrDay_st))
                            {
                                QString filename;

                                filename.sprintf("%s%s\\%s_%.2u_%.2u_%.4u.trn",ss->trend_path,trend->m_objectName.toStdString().c_str(),
                                                 trend->m_trendName.toStdString().c_str(),rrDay_st,rrMonth_st,rrYear_st);

                                QFile trend_file(filename);

                                if (!trend_file.exists())
                                {
                                    if (trend_file.open(QIODevice::ReadWrite))
                                    {
                                        trend_file.write((char *)ss->empty_file,69120);
                                        trend_file.close();
                                        //trend_file.waitForBytesWritten(5000);
                                    }

                                }
                                trend_file.open(QIODevice::ReadWrite);


                                trend_file.read((char *)buff_file,69120);


                                int Razmer_faila=17280;
                                if (rrYear_st==now_time.date().year() && rrMonth_st==now_time.date().month() && rrDay_st==now_time.date().day())
                                {Razmer_faila=Position_end;}
                                bool nachalo_naideno=false;
                                int pos_nach=0,pos_kon=0;
                                for (int j=0;j<Razmer_faila;j++)
                                {
                                    if (!nachalo_naideno && buff_file[j]==ss->min_float) {nachalo_naideno=true;pos_nach=j;}

                                    if (  (nachalo_naideno && buff_file[j]!=ss->min_float) || (nachalo_naideno && j==Razmer_faila-1)
                                            || (nachalo_naideno && j-pos_nach>200) )
                                    {
                                        struct zapros
                                        {
                                            char FileName[32];
                                            int Year,Month,Day;
                                            int Position;
                                            int Count;
                                        } zapr;

                                        if (nachalo_naideno && buff_file[j]!=ss->min_float) {aligner=0;}
                                        if ((nachalo_naideno && j==Razmer_faila-1) || (nachalo_naideno && j-pos_nach>200)) {aligner=1;}


                                        pos_kon=j;
                                        nachalo_naideno=false;
                                        strcpy(zapr.FileName,trend->m_trendName.toStdString().c_str());
                                        zapr.Year=rrYear_st;
                                        zapr.Month=rrMonth_st;
                                        zapr.Day=rrDay_st;
                                        zapr.Position=pos_nach;
                                        zapr.Count=pos_kon-pos_nach+aligner;

                                        //qDebug() << "zapr: " << zapr.FileName;
                                        //qDebug() << "zapr: " << zapr.Year;
                                        //qDebug() << "zapr: " << zapr.Month;
                                        //qDebug() << "zapr: " << zapr.Day;
                                        //qDebug() << "zapr: " << zapr.Position;
                                        //qDebug() << "zapr: " << zapr.Count;

                                        float buff_new[300];
                                        int result;
                                        result=repl_socket.write((char *)&zapr,sizeof(zapr));

                                        if (result!=SOCKET_ERROR)
                                        {
                                            repl_socket.waitForReadyRead(5000);
                                            result=repl_socket.read((char *)buff_new,(zapr.Count)*4);

                                            if (result==zapr.Count*4)
                                            {
                                                trend_file.seek(pos_nach*4);//,CFile::begin);
                                                trend_file.write((char *)buff_new,(zapr.Count)*4);
                                                //trend_file.Flush();
                                                if (CheckThreadStop()){qDebug() << "111111111111111111111111111111111111111111"; return;}
                                            }
                                        }
                                    }
                                }
                                trend_file.close();
                                if (CheckThreadStop()) {qDebug() << "2222222222222222222222222222222222222222222"; return;}
                            }
                        }
                        repl_socket.close();
                        if (CheckThreadStop()) {qDebug() << "3333333333333333333333333333333333333333333333"; return;}
                }
            }

            if (need_restart_repl==true) {cycles_cnt=0;need_restart_repl=false;}
        }

    emit textchange_repl(this->m_this_number,this->m_nameObject, "");

    return;

}

//=========================================================================================

VirtualNode::VirtualNode(int this_number,QString objectName,QString objectType,
                          QString IP_addr, uint port,
                          uint port_repl,uint port_local,
                          uint modbus_start_address,
                          uint num_float_tags)
{
    m_this_number=this_number;

    m_nameObject=objectName;
    m_typeObject=objectType;
    m_IP_addr=IP_addr;
    m_port=port;
    m_port_repl=port_repl;
    m_port_local=port_local;
    m_modbus_start_address=modbus_start_address;
    m_srv.num_float_tags=num_float_tags;

   // emit textchange(this->m_this_number,this->m_nameObject,  "running");
   // start();
    connect(&timerCalculateVariables,SIGNAL(timeout()),this,SLOT(TimerCalculateVariablesEvent()));

    timerCalculateVariables.start(1000);

}
//=========================================================================================
VirtualNode::~VirtualNode()
{
    timerCalculateVariables.stop();  //- error QObject::killTimers: timers cannot be stopped from another thread
}
//=========================================================================================
void VirtualNode::TimerCalculateVariablesEvent()
{

    //убедимся что все узлы необходимые для этого вирт.контроллера присутствуют
    //QVector<virt_tag_struct> vectVirtTags
    bool allControllersConnected=true;
    foreach(virt_tag_struct virtTag,vectVirtTags)
    {
        foreach(virt_expr_member_struct exprMember, virtTag.vectVirtTagExprMembers)
        {
            if (exprMember.objectName!=this->m_nameObject) // ход конем чтоб можно было использовать свои же тэги, в противном случае он будет ждать себя же
            {
                if (!ss->hashCommonNodes[exprMember.objectName]->m_isReaded) allControllersConnected=false;
            }
        }
    }

    //считаем значения
    if (allControllersConnected)
    {
        float tmp_TagValue=0.0;

        foreach(virt_tag_struct virtTag,vectVirtTags)
        {
            QString tmp_virtTagExpression;
            tmp_virtTagExpression=virtTag.virtTagExpression;

            foreach(virt_expr_member_struct exprMember, virtTag.vectVirtTagExprMembers)
            {
                //Logger::Instance()->AddLog(exprMember.objectName+","+QString::number(exprMember.numInBuff),Qt::black);


                tmp_virtTagExpression.replace(exprMember.objectName+"["+QString::number(exprMember.numInBuff)+"]",
                                              QString::number(ss->hashCommonNodes[exprMember.objectName]->m_srv.buff[exprMember.numInBuff]));
            }

            tmp_TagValue=virtControllerScriptEngine.evaluate(tmp_virtTagExpression).toNumber();

            //Logger::Instance()->AddLog(virtTag.virtTagExpression+ "="+tmp_virtTagExpression+"="+QString::number(tmp_TagValue),Qt::black);

            if ( tmp_TagValue!=tmp_TagValue)  //тривиальная проверка на NaN
            {
                   tmp_TagValue=0.0;
            }

            this->m_srv.buff[virtTag.numInBuff]=tmp_TagValue;

        }


        if (m_isConnected==false)
        {
            emit textchange(this->m_this_number,this->m_nameObject,  "connected");
            emit textSave2LogFile(this->m_this_number,this->m_nameObject,  "connected");
        }
        m_isConnected=true;
        m_isReaded=true;
    }
    else
    {
        if (m_isConnected==true)
        {
            emit textchange(this->m_this_number,this->m_nameObject,  "disconnected");
            emit textSave2LogFile(this->m_this_number,this->m_nameObject,  "disconnected");
        }
        m_isConnected=false;
        m_isReaded=false;
    }


}
//=========================================================================================
