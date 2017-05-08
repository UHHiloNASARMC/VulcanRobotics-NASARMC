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
    bool m_forceReconnect = false;
public:
    SpockTCPSocket(const QString& hostname, MainWindow* mainWindow);
    void reestablishConnection();
    void forceReconnect() { m_forceReconnect = true; }
    std::pair<SpockStatusData, bool> getStatusData();
    void sendPacket(const SpockCommandData& data);
    QString getHostAddress() const;
public slots:
    void dnsHostFound(const QHostInfo& host);
private:
    QString m_hostname;
};

#endif // SPOCKTCPSOCKET_H
