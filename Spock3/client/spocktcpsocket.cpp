#include "spocktcpsocket.h"
#include "mainwindow.h"
#include <QHostInfo>

SpockTCPSocket::SpockTCPSocket(const QString& hostname, MainWindow* mainWindow)
: QTcpSocket(mainWindow), m_hostname(hostname)
{
    setSocketOption(ReceiveBufferSizeSocketOption, 128);
    connect(this, SIGNAL(connected()), mainWindow, SLOT(connectionEstablished()));
    connect(this, SIGNAL(disconnected()), mainWindow, SLOT(connectionLost()));
    reestablishConnection();
}

void SpockTCPSocket::reestablishConnection()
{
    //printf("state: %d\n", state());
    //fflush(stdout);
    if (!m_doingDns && state() != ConnectedState && state() != ConnectingState)
    {
        QHostInfo::lookupHost(m_hostname, this, SLOT(dnsHostFound(QHostInfo)));
        m_doingDns = true;
    }
}

void SpockTCPSocket::dnsHostFound(const QHostInfo& host)
{
    m_doingDns = false;
    for (const QHostAddress& addr : host.addresses())
    {
        if (addr.protocol() == IPv4Protocol)
        {
            connectToHost(addr, 5555);
            break;
        }
    }
}

SpockStatusData SpockTCPSocket::getStatusData()
{
    qint64 avail = bytesAvailable();
    //printf("avail %lld\n", avail);
    //fflush(stdout);
    while (avail >= 62)
    {
        QByteArray bytes = read(62);
        if (bytes.size() >= 62)
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
        }
        avail = bytesAvailable();
    }

    return m_cachedData;
}

void SpockTCPSocket::sendPacket(const SpockCommandData& data)
{
    if (state() == ConnectedState)
    {
        QByteArray bytes;
        bytes.reserve(35);
        QDataStream s(&bytes, WriteOnly);
        s.writeRawData("RoboComm", 8);
        s << quint64(time(nullptr));
        s << ++m_lastPacketSent;
        data.write(s);
        qint64 written = write(bytes);
        printf("%lld %lld\n", written, m_lastPacketSent);
    }
}

QString SpockTCPSocket::getHostAddress() const
{
    return peerAddress().toString();
}
