#ifndef ODBCDB_H
#define ODBCDB_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QMessageBox>
#include <QMutex>
#include <QDateTime>
#include "logger.h"

class OdbcDb
{
private:
//    QSqlDatabase *db;
    QString odbcName;
    QString user;
    QString pass;
    unsigned int counter;
    QMutex counterMutex;
    QString GetNextName();
public:
    Logger *logger;
    OdbcDb(QString odbcName,QString user, QString pass);
    ~OdbcDb();
    bool ExecQuery(QString query);
    void AddAlarm2DB(QString what);

};

#endif // ODBCDB_H
