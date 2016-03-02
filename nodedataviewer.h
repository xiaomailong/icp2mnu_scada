#ifndef NODEDATAVIEWER_H
#define NODEDATAVIEWER_H

#include <QDialog>
#include <QTimer>
#include <QString>
#include <QHash>

#include "nodes.h"

namespace Ui {
class NodeDataViewer;
}

class NodeDataViewer : public QDialog
{
    Q_OBJECT

public:
    explicit NodeDataViewer(CommonNode *node, QVector<CommonTrend *> *vectTrends, QWidget *parent = 0);
    ~NodeDataViewer();
    QTimer m_timer1s;
    CommonNode *m_node;
    void closeEvent(QCloseEvent *event);
    QHash<int, QString> hashTagNames;
    QString FormattedTagString(QString objName, uint numInBuff, float tagValue, QString tagName);

public slots:
    void Timer1sEvent();

private:
    Ui::NodeDataViewer *ui;
};

#endif // NODEDATAVIEWER_H
