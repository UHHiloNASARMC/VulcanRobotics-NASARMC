#ifndef SPOCKTCPSOCKET_H
#define SPOCKTCPSOCKET_H

#include <QTcpSocket>
#include "spockcommon.h"

class MainWindow;
class QHostInfo;

class SpockTCPSocket : public QTcpSocket
{
    Q_OBJECT
    SpockStatusData m_cachedData;
    quint64 m_lastReceivedPacket = 0;
    quint64 m_lastPacketSent = 0;
    bool m_doingDns = false;
public:
    SpockTCPSocket(const std::string& hostname, MainWindow* mainWindow);
    void reestablishConnection();
    SpockStatusData getStatusData();
    void sendPacket(const SpockCommandData& data);
    QString getHostAddress() const;
public slots:
    void dnsHostFound(const QHostInfo& host);
private:
    std::string m_hostname;
};

#endif // SPOCKTCPSOCKET_H
