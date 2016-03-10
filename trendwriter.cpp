#include "trendwriter.h"

//=========================================================================================
TrendWriter::TrendWriter()
{
    ss=ScadaServer::GetInstance();
}
//========================================================================================
void TrendWriter::FuncFileWriter(CommonTrend *this_trend_info, char *str_date, uint time_pos)
{
    QString filename;

    filename.sprintf("%s%s\\%s_%s.trn",ss->trend_path,this_trend_info->m_objectName.toStdString().c_str(),this_trend_info->m_trendName.toStdString().c_str(),str_date);

    QFile trend(filename);

    if (!trend.exists())  //Open(filename,CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyNone,NULL))
        {

            if (trend.open(QIODevice::ReadWrite))
            {
                trend.write((char *)ss->empty_file,69120);
                trend.seek(time_pos*4);//, CFile::begin);
                trend.write((char *)&(ss->hashCommonNodes[this_trend_info->m_objectName]->m_srv.buff[this_trend_info->m_numInBuff]),4);
                trend.close();
            }
        }
        else
        {
            trend.open(QIODevice::ReadWrite);
            trend.seek(time_pos*4);//, CFile::begin);
            trend.write((char *)&(ss->hashCommonNodes[this_trend_info->m_objectName]->m_srv.buff[this_trend_info->m_numInBuff]),4);
            trend.close();
        }

    return;
}

//=========================================================================================
void TrendWriter::run()
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
            snprintf(buf_date,20,"%.2u_%.2u_%.4u",currDateTime.date().day(),currDateTime.date().month(),currDateTime.date().year());

            foreach (CommonTrend *tr,ss->vectCommonTrends)
            {

                if (ss->hashCommonNodes.contains(tr->m_objectName) && ss->hashCommonNodes[tr->m_objectName]->m_isReaded)
                {
                    FuncFileWriter(tr,buf_date,time22);
                }
            }
        }
        iprevtime=itime;
        if (CheckThreadStop()) return;
    }

}
