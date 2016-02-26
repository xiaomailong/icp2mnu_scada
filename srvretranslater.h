#ifndef SRVRETRANSLATER_H
#define SRVRETRANSLATER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

class SrvReTranslater : public QObject
{
    Q_OBJECT

    public:
        SrvReTranslater();
        virtual ~SrvReTranslater();
    float buff[100];
    unsigned int num_float_tags; //1 float tag = 2 modbus registers



    public slots:
        // Slot to handle disconnected client
        void ClientDisconnected();

    private slots:
        // New client connection
        void NewConnection();

    public:
        QTcpServer* m_pServerSocket;
        QList<QTcpSocket*> m_pClientSocketList;
        //QTcpSocket* lastClient;
};


#endif // SRVRETRANSLATER_H
