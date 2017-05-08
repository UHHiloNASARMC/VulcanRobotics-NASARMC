#include "spocktcpsocket.h"
#include "mainwindow.h"
#include <QHostInfo>

SpockTCPSocket::SpockTCPSocket(const QString& hostname, MainWindow* mainWindow)
: QTcpSocket(mainWindow), m_hostname(hostname)
{
    setSocketOption(ReceiveBufferSizeSocketOption, 64);
    connect(this, SIGNAL(connected()), mainWindow, SLOT(connectionEstablished()));
    connect(this, SIGNAL(disconnected()), mainWindow, SLOT(connectionLost()));
    reestablishConnection();
}

void SpockTCPSocket::reestablishConnection()
{
    //printf("state: %d\n", state());
    //fflush(stdout);
    if (!m_doingDns && (m_forceReconnect || (state() != ConnectedState && state() != ConnectingState)))
    {
        QHostInfo::lookupHost(m_hostname, this, SLOT(dnsHostFound(QHostInfo)));
        m_doingDns = true;
        m_forceReconnect = false;
    }
}

void SpockTCPSocket::dnsHostFound(const QHostInfo& host)
{
    m_doingDns = false;
    for (const QHostAddress& addr : host.addresses())
    {
        if (addr.protocol() == IPv4Protocol)
        {
            disconnectFromHost();
            waitForDisconnected(1000);
            connectToHost(addr, 5555);
            //printf("CONNECT\n");
            break;
        }
    }
}

std::pair<SpockStatusData, bool> SpockTCPSocket::getStatusData()
{
    //waitForReadyRead(1000);
    qint64 avail = bytesAvailable();
    //printf("avail %lld\n", avail);
    //fflush(stdout);
    bool received = false;
    while (avail >= 64)
    {
        QByteArray bytes = read(64);
        if (bytes.size() >= 64)
        {
            QDataStream s(bytes);
            char magic[8];
            s.readRawData(magic, 8);
            quint64 timestamp, packetCount;
            s >> timestamp;
            s >> packetCount;
            if (!memcmp(magic, "RoboStat", 8))
            {
                m_lastReceivedPacket = packetCount;
                m_cachedData.read(s);
            }
            received = true;
            //printf("Received %lld\n", packetCount);
        }
        avail = bytesAvailable();
    }

    return {m_cachedData, received};
}

void SpockTCPSocket::sendPacket(const SpockCommandData& data)
{
    if (state() == ConnectedState)
    {
        QByteArray bytes;
        bytes.reserve(64);
        QDataStream s(&bytes, WriteOnly);
        s.writeRawData("RoboComm", 8);
        s << quint64(time(nullptr));
        s << ++m_lastPacketSent;
        data.write(s);
        bytes.append(29, '\0');
        size_t written = write(bytes);
        //printf("Send %lld %lld\n", written, m_lastPacketSent);
    }
}

QString SpockTCPSocket::getHostAddress() const
{
    return peerAddress().toString();
}
