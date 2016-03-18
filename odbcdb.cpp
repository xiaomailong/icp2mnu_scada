#include "odbcdb.h"


//======================================================================
OdbcDb::OdbcDb(QString odbcName,QString user, QString pass)
{
    logger=Logger::Instance();
    this->odbcName=odbcName;
    this->user=user;
    this->pass=pass;
    counter=0;
}
//=======================================================================
OdbcDb::~OdbcDb()
{
}
//=======================================================================
QString OdbcDb::GetNextName()
{
    QString res;
    counterMutex.lock();
    res.sprintf("%u",counter);
    counter=(counter+1) % 1000000;
    counterMutex.unlock();
    return res;
}
//=======================================================================
bool OdbcDb::ExecQuery(QString query)
{
    bool res=true;
    QString connectionName=GetNextName();


    {
        // start of the block where the db object lives

        QSqlDatabase db = QSqlDatabase::addDatabase("QODBC",connectionName);

        db.setDatabaseName(odbcName);
        db.setUserName(user);
        db.setPassword(pass);

        if (db.open())
        {
            QSqlQuery sqlQuery(db);
            sqlQuery.exec(query);

            if (sqlQuery.lastError().isValid())
            {
                logger->AddLog("ODBC: Ошибка "+sqlQuery.lastError().text(),Qt::red);
                res=false;
            }

        }
        else
        {
            logger->AddLog("ODBC: Ошибка "+db.lastError().text(),Qt::red);
            res=false;
        }
        logger->AddLog("ODBC: Выполнено :"+query,Qt::darkGreen);
        db.close();
    } // end of the block where the db object lives, it will be destroyed here

    QSqlDatabase::removeDatabase(connectionName);

    return res;
}
//=======================================================================
void OdbcDb::AddAlarm2DB(QString what)
{
    QString strDateTime;
    QDateTime dt;
    dt=QDateTime::currentDateTime();
    strDateTime.sprintf("%.2u.%.2u.%.4u %.2u:%.2u:%.2u.%.3u",
                        dt.date().day(),dt.date().month(),dt.date().year(),
                        dt.time().hour(),dt.time().minute(),dt.time().second(),dt.time().msec());


    ExecQuery("INSERT INTO ALARMS(DT,ALARMTEXT) VALUES('" + strDateTime + "', '" + what + "')");


}
//============================================================================
/*
CREATE TABLE ALARMS
(
ID INTEGER,
DT TIMESTAMP,
ALARMTEXT VARCHAR(128),
PRIMARY KEY(ID)
);
+ autoincrement generator trigger on ID
*/
//============================================================================
