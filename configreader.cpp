#include "configreader.h"
#include "nodes.h"
#include "logger.h"



/*
Pattern для конфигурации серверов nodes.conf
(\w+)\s+(modbus|mnu_scada)\s+(\d+\.\d+\.\d+\.\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)


к паттерну можно добавить \n или $(конейц строки), можно заменить \s+ (любое кол-во "пробельных" [\r\n\t\f ] символов) на [\t ]+ (только табуляция и пробел)

String
lelaki	modbus 172.16.9.243	502	0	6050	200	11


Captures:
1.	[0-6]	`lelaki`
2.	[7-13]	`modbus`
3.	[14-26]	`172.16.9.243`
4.	[27-30]	`502`
5.	[31-32]	`0`
6.	[33-37]	`6050`
7.	[38-41]	`200`
8.	[42-44]	`11`


с сайта https://regex101.com/ - тут можно посмотреть детализированные объяснения
*/




ConfigReader::ConfigReader()
{
    foundSectionNodes=false;
    foundSectionTrends=false;
}
//================================================================
void ConfigReader::SetConfigFileName(QString fileName)
{
    configFile.setFileName(fileName);
}
//================================================================
bool ConfigReader::OpenConfig()
{
    foundSectionNodes=false;
    foundSectionTrends=false;
    foundSectionAlarms=false;
    foundSectionVirtualControllers=false;
    return configFile.open(QIODevice::ReadOnly | QIODevice::Text);
}
//================================================================
void ConfigReader::SeekStartConfig()
{
    configFile.seek(0);
}
//================================================================
void ConfigReader::CloseConfig()
{
    configFile.close();
}
//================================================================
bool ConfigReader::ReadNextNode(QString &objectName,QString &objectType,QString &IP_addr,
                                       uint &port,uint &port_repl,uint &port_local,
                                       uint &modbus_start_address,uint &num_float_tags)
{
    if (!foundSectionNodes)
    {
        while(!configFile.atEnd())
        {
            QString node_conf_line=configFile.readLine();
            if (node_conf_line.left(strlen("[NODES]"))=="[NODES]")
            {
                foundSectionNodes=true;
                break;
            }
        }
    }

    while(!configFile.atEnd())
    {
        QString node_conf_line=configFile.readLine();
        //logger->AddLog("Node File: "+node_conf_line,Qt::black);

        QRegExp patternEmptyStr("^\\s*\\n*$");

        if (node_conf_line[0]=='#' || patternEmptyStr.indexIn(node_conf_line)!=-1) continue;   //комментарии или пустые строки

        if (node_conf_line[0]=='[') return false;   //достигнута следующая секция

        QRegExp regExp("^(\\w+)\\s+(modbus|mnu_scada|virtual)\\s+(\\d+\\.\\d+\\.\\d+\\.\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)");  //   \\s?\\n?$");
        if(regExp.indexIn(node_conf_line)!=-1)  //неподходящие строки игнорируем и выводим в лог
        {
            //for(int i=1;i<9;++i)
            //logger->AddLog("Node File:      " + regExp.cap(i),Qt::black);

            objectName=regExp.cap(1);
            objectType=regExp.cap(2);
            IP_addr=regExp.cap(3);
            port=regExp.cap(4).toUInt();
            port_repl=regExp.cap(5).toUInt();
            port_local=regExp.cap(6).toUInt();
            modbus_start_address=regExp.cap(7).toUInt();
            num_float_tags=regExp.cap(8).toUInt();

            return true;
        }
        else
        {
            Logger::Instance()->AddLog("Config Node Error Format: "+node_conf_line,Qt::red);
        }
    }

    return false;    //достигнут конец файла

}
//================================================================
bool ConfigReader::ReadNextTrend(QString &objectName, QString &trendName,uint &numInBuff)
{
    if (!foundSectionTrends)
    {
        while(!configFile.atEnd())
        {
            QString node_conf_line=configFile.readLine();
            if (node_conf_line.left(strlen("[TRENDS]"))=="[TRENDS]")
            {
                foundSectionTrends=true;
                break;
            }
        }
    }

    while(!configFile.atEnd())
    {
        QString node_conf_line=configFile.readLine();
        //logger->AddLog("Node File: "+node_conf_line,Qt::black);

        QRegExp patternEmptyStr("^\\s*\\n*$");

        if (node_conf_line[0]=='#' || patternEmptyStr.indexIn(node_conf_line)!=-1) continue;   //комментарии или пустые строки

        if (node_conf_line[0]=='[') return false;   //достигнута следующая секция

        QRegExp regExp("^(\\w+)\\s+(\\w+)\\s+(\\d+)");  //   \\s?\\n?$");
        if(regExp.indexIn(node_conf_line)!=-1)  //неподходящие строки игнорируем и выводим в лог
        {
            //for(int i=1;i<9;++i)
            //logger->AddLog("Node File:      " + regExp.cap(i),Qt::black);

            objectName=regExp.cap(1);
            trendName=regExp.cap(2);
            numInBuff=regExp.cap(3).toUInt();


            return true;
        }
        else
        {
            Logger::Instance()->AddLog("Config Trend Error Format: "+node_conf_line,Qt::red);
        }
    }

    return false;    //достигнут конец файла
}
//================================================================
//alarm pattern
//^(information|warning|critical|connect)\s+([\w\[\]+-]+)([<>=]?)([+-]?\d*\.?\d*)\s+(\d*)\s+(.+)$
//2 вида алармов по типу:
//1. алармі связи, вид - connect talal 10 Аларм по соединеие с талал, задержка 10с
// при таком аларме вектор пустой
//2. warning talal[4]-ggpz_gz[5]>10.0 180 Разница больше 10,задержка 180с.
//вектор содержит группы знак (+,-), имя узла, номер в буфере узла - из этого складывается результат
bool ConfigReader::ReadNextAlarm(QString &alarmType, QString &alarmExpression, QVector<alarm_expr_member_struct> &alarmVectExprMembers,
                   QString &alarmComparison, float &alarmValue, uint &alarmDelay_s, QString &alarmText)
{
    if (!foundSectionAlarms)
    {
        while(!configFile.atEnd())
        {
            QString node_conf_line=configFile.readLine();
            if (node_conf_line.left(strlen("[ALARMS]"))=="[ALARMS]")
            {
                foundSectionAlarms=true;
                break;
            }
        }
    }

    while(!configFile.atEnd())
    {
        QString node_conf_line=configFile.readLine();
        //logger->AddLog("Node File: "+node_conf_line,Qt::black);

        QRegExp patternEmptyStr("^\\s*\\n*$");

        if (node_conf_line[0]=='#' || patternEmptyStr.indexIn(node_conf_line)!=-1) continue;   //комментарии или пустые строки

        if (node_conf_line[0]=='[') return false;   //достигнута следующая секция

        QRegExp regExp("^(information|warning|critical|connect)\\s+([\\w\\d\\[\\]+-\\/*\\(\\)]+)([<>=]?)([+-]?\\d*\\.?\\d*)\\s+(\\d*)\\s+(\\S.+[^\\n])\\n*$");
        if(regExp.indexIn(node_conf_line)!=-1)  //неподходящие строки игнорируем и выводим в лог
        {
            alarmType=regExp.cap(1);
            if (alarmType=="connect")
            {
                alarmExpression=regExp.cap(2);
                alarmComparison="=";
                alarmValue=0;
                alarmDelay_s=regExp.cap(5).toUInt();
                alarmText=regExp.cap(6);
            }
            else    //information|warning|critical
            {
                alarmExpression=regExp.cap(2);
                QString expr_members_line=alarmExpression;
                alarmVectExprMembers.clear();

                QRegExp regExpExprMembers("(\\w+)\\[(\\d+)\\]");  //пример: talal[5]   ,  ggpz_gaz[6]   и т.д.
                while(regExpExprMembers.indexIn(expr_members_line)!=-1)
                {
                    alarm_expr_member_struct member;

                    member.objectName=regExpExprMembers.cap(1);
                    member.numInBuff=regExpExprMembers.cap(2).toUInt();

                    //отрезать с учетом функций!!!!, т.е. talal[5] может идти не с начала (Math.abs(talal[5]-ggpz_gaz[6])

                    expr_members_line=expr_members_line.right(expr_members_line.length() - regExpExprMembers.pos(1) -
                                                              regExpExprMembers.cap(1).length() -
                                                              regExpExprMembers.cap(2).length() - 2);  //2 - two brackets []


                    alarmVectExprMembers.append(member);
                }

                //массив отсортировать по убыванию  длины member.objectName
                 // ЗАЧЕМ: чтобы исключить замену в длинных именах объектов боллее коротких частей (тоже имен обектов)
                 //например "yaroshiv[1]-talal_z_yaroshiv[1]" при подстановке цифры вместо yaroshiv[1]
                 //даст 12.568-talal_z_12.568
                 //поэтому сначала заменять более длинные имена, talal_z_yaroshiv

                qSort(alarmVectExprMembers);
                //лямбду не принимает -
                //qSort(alarmVectExprMembers.begin(),alarmVectExprMembers.end(),
                //        [](const alarm_expr_member_struct &a, const alarm_expr_member_struct &b) { return a.objectName.length() > b.objectName.length(); });

                alarmComparison=regExp.cap(3);
                alarmValue=regExp.cap(4).toFloat();
                alarmDelay_s=regExp.cap(5).toUInt();
                alarmText=regExp.cap(6);
            }

            return true;
        }
        else
        {
            Logger::Instance()->AddLog("Config Alarm Error Format: "+node_conf_line,Qt::red);
        }
    }

    return false;    //достигнут конец файла


}
//================================================================
bool ConfigReader::ReadNextVirtualTag(QString &objectName,uint &numInBuff,QString &virtTagExpression, QVector<virt_expr_member_struct> &virtTagVectExprMembers)
{
    if (!foundSectionVirtualControllers)
    {
        while(!configFile.atEnd())
        {
            QString node_conf_line=configFile.readLine();
            if (node_conf_line.left(strlen("[VIRTUAL_CONTROLLERS]"))=="[VIRTUAL_CONTROLLERS]")
            {
                foundSectionVirtualControllers=true;
                break;
            }
        }
    }

    while(!configFile.atEnd())
    {
        QString node_conf_line=configFile.readLine();
        //logger->AddLog("Node File: "+node_conf_line,Qt::black);

        QRegExp patternEmptyStr("^\\s*\\n*$");

        if (node_conf_line[0]=='#' || patternEmptyStr.indexIn(node_conf_line)!=-1) continue;   //комментарии или пустые строки

        if (node_conf_line[0]=='[') return false;   //достигнута следующая секция

        QRegExp regExp("^(\\w+)\\s+(\\d+)\\s+([\\w\d\\[\\]+-\\/*\\(\\)]+)");  //   \\s?\\n?$");
        if(regExp.indexIn(node_conf_line)!=-1)  //неподходящие строки игнорируем
        {
            objectName=regExp.cap(1);
            numInBuff=regExp.cap(2).toUInt();
            virtTagExpression=regExp.cap(3);
            QString expr_members_line=virtTagExpression;
            virtTagVectExprMembers.clear();

            QRegExp regExpExprMembers("(\\w+)\\[(\\d+)\\]");  //пример: talal[5]   ,  ggpz_gaz[6]   и т.д.
            while(regExpExprMembers.indexIn(expr_members_line)!=-1)
            {
                virt_expr_member_struct member;

                member.objectName=regExpExprMembers.cap(1);
                member.numInBuff=regExpExprMembers.cap(2).toUInt();

                //отрезать с учетом функций!!!!, т.е. talal[5] может идти не с начала (Math.abs(talal[5]-ggpz_gaz[6])

                expr_members_line=expr_members_line.right(expr_members_line.length() - regExpExprMembers.pos(1) -
                                                          regExpExprMembers.cap(1).length() -
                                                          regExpExprMembers.cap(2).length() - 2);  //2 - two brackets []


                virtTagVectExprMembers.append(member);
            }

            //массив отсортировать по убыванию  длины member.objectName
             // ЗАЧЕМ: чтобы исключить замену в длинных именах объектов боллее коротких частей (тоже имен обектов)
             //например "yaroshiv[1]-talal_z_yaroshiv[1]" при подстановке цифры 12.568 вместо yaroshiv[1]
             //даст 12.568-talal_z_12.568
             //поэтому сначала заменять более длинные имена, talal_z_yaroshiv

            qSort(virtTagVectExprMembers);
            //лямбду не принимает -
            //qSort(virtTagVectExprMembers.begin(),virtTagVectExprMembers.end(),
            //        [](const virt_tag_expr_member_struct &a, const virt_tag_expr_member_struct &b) { return a.objectName.length() > b.objectName.length(); });

            return true;
        }
        else
        {
            Logger::Instance()->AddLog("Config Virt Controller Error Format: "+node_conf_line,Qt::red);
        }
    }

    return false;    //достигнут конец файла
}
//================================================================
