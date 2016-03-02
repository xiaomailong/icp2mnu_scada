#include "nodedataviewer.h"
#include "ui_nodedataviewer.h"


//=====================================================
//=====================================================
NodeDataViewer::NodeDataViewer(CommonNode *node, QVector<CommonTrend *> *vectTrends, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodeDataViewer)
{
    m_node=node;
    ui->setupUi(this);
    connect(&m_timer1s,SIGNAL(timeout()),this,SLOT(Timer1sEvent()));
    connect(ui->closeButton,SIGNAL(clicked()),this,SLOT(close()));

    //заполним хэш соответствия номеров тэгов и их имен для этого конкретного объекта
    foreach(CommonTrend* trend, *vectTrends)
    {
        if (node->m_nameObject==trend->m_objectName)
        {
            hashTagNames.insert(trend->m_numInBuff,trend->m_trendName);
        }
    }

    for(uint i=0;i<m_node->m_srv.num_float_tags;++i)
    {
        if (hashTagNames.contains(i))
            ui->listWidget->addItem(FormattedTagString(m_node->m_nameObject, i, m_node->m_srv.buff[i], hashTagNames[i]));
        else
            ui->listWidget->addItem(FormattedTagString(m_node->m_nameObject, i, m_node->m_srv.buff[i], ""));
    }

    m_timer1s.start(1000);
}
//=====================================================
NodeDataViewer::~NodeDataViewer()
{
    delete ui;
}
//=====================================================
QString NodeDataViewer::FormattedTagString(QString objName, uint numInBuff, float tagValue, QString tagName)
{
    char tag_text[64];
    for (uint i=0;i<64;++i) tag_text[i]=' ';
    tag_text[64]=0;


    strcpy(&tag_text[0],  (objName+"["+QString::number(numInBuff)+"] = ").toStdString().c_str());
    tag_text[strlen(tag_text)]=' '; //уберем добавленный конец строки
    strcpy(&tag_text[objName.length()+7], QString::number(tagValue).toStdString().c_str());
    tag_text[strlen(tag_text)]=' ';
    strcpy(&tag_text[objName.length()+20], tagName.toStdString().c_str());

    return QString(tag_text);
}
//=====================================================
void NodeDataViewer::Timer1sEvent()
{
    for(uint i=0;i<m_node->m_srv.num_float_tags;++i)
    {
        if (hashTagNames.contains(i))
            ui->listWidget->item(i)->setText(FormattedTagString(m_node->m_nameObject, i, m_node->m_srv.buff[i], hashTagNames[i]));
        else
            ui->listWidget->item(i)->setText(FormattedTagString(m_node->m_nameObject, i, m_node->m_srv.buff[i], ""));
    }

}
//=====================================================
void NodeDataViewer::closeEvent(QCloseEvent *event)
{
    m_timer1s.stop();
    //qDebug() << "del.....";
    deleteLater();
}
//=====================================================

