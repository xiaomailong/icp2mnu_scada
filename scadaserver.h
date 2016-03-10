#ifndef SCADASERVER_H
#define SCADASERVER_H

#include <QObject>
#include <QMutex>
#include <QVector>
#include <QHash>
//#include "nodes.h"
#include "alarm.h"
#include "trendwriter.h"

class CommonNode;
class CommonTrend;
class Alarms;
class TrendWriter;
struct alarm_tag_struct;

class ScadaServer: public QObject
{
    Q_OBJECT
private:
    ScadaServer();
    ~ScadaServer();

    static ScadaServer* theSingleInstanceScadaServer;

    static QMutex mutex;

public:
    static ScadaServer* GetInstance();

    static const float min_float=-3.4028234663852886e+38;       //минимальное число float, используется как "пустое" значение,
                                                                //для отличия от NaN которое обозначает результат некоректного скриптового выражения
    char *trend_path;//="d:\\MNU_SCADA\\trends\\";   //C++11 'constexpr' only available with -std=c++11 or -std=gnu++11
    float empty_file[17280];
    //Подсистемы СКАДы
    //АЛАРМЫ
    Alarms *alarms;
    QVector<alarm_tag_struct> vectAlarmTags;

    QHash<QString, CommonNode *> hashCommonNodes;
    QVector<CommonTrend *> vectCommonTrends;
    TrendWriter *trendWriterThread;


};

#endif // SCADASERVER_H
