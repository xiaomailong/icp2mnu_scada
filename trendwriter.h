#ifndef TRENDWRITER_H
#define TRENDWRITER_H
#include "autostopthread.h"
#include "scadaserver.h"
#include "nodes.h"

class ScadaServer;
class CommonTrend;

class TrendWriter: public AutoStopThread
{
    Q_OBJECT
private:
    ScadaServer *ss;
    void run();
    void FuncFileWriter(CommonTrend *this_trend_info, char *str_date, uint time_pos);
public:
    TrendWriter();

};

#endif // TRENDWRITER_H
