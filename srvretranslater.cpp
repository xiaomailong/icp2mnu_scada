#include "srvretranslater.h"


SrvReTranslater::SrvReTranslater()
{
    m_pServerSocket = new QTcpServer(this);
    connect(m_pServerSocket, SIGNAL(newConnection()), this, SLOT(NewConnection()));
}
//==================================================================================
SrvReTranslater::~SrvReTranslater()
{
    delete m_pServerSocket;
}
//==================================================================================
void SrvReTranslater::NewConnection()
{
    QTcpSocket* pClient = m_pServerSocket->nextPendingConnection();
    pClient->setSocketOption(QAbstractSocket:: KeepAliveOption, 1);
    pClient->write((char *)buff,num_float_tags*4);
    m_pClientSocketList.push_back(pClient);

    connect(pClient, SIGNAL(disconnected()), this, SLOT(ClientDisconnected()));
    //lastClient=pClient;
}
//======================================================================================
void SrvReTranslater::ClientDisconnected()
{
    // client has disconnected, so remove from list
    QTcpSocket* pClient = static_cast<QTcpSocket*>(QObject::sender());
    m_pClientSocketList.removeOne(pClient);
}
//===========================================================================================================
