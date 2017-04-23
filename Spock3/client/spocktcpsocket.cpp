#include "spocktcpsocket.h"
#include "mainwindow.h"
#include <QHostInfo>

SpockTCPSocket::SpockTCPSocket(const std::string& hostname, MainWindow* mainWindow)
: m_hostname(hostname)
{
    setSocketOption(ReceiveBufferSizeSocketOption, 61);
    connect(this, SIGNAL(connected()), mainWindow, SLOT(connectionEstablished()));
    connect(this, SIGNAL(disconnected()), mainWindow, SLOT(connectionLost()));
    reestablishConnection();
}

void SpockTCPSocket::reestablishConnection()
{
    if (state() != ConnectedState && state() != ConnectingState)
    {
        QHostInfo nsLookup = QHostInfo::fromName(m_hostname.c_str());
        for (const QHostAddress& addr : nsLookup.addresses())
        {
            if (addr.protocol() == IPv4Protocol)
            {
                connectToHost(addr, 5555);
                break;
            }
        }
    }
}

SpockStatusData SpockTCPSocket::getStatusData()
{
    while (bytesAvailable() >= 61)
    {
        QByteArray bytes = read(61);
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
        write(bytes);
    }
}

QString SpockTCPSocket::getHostAddress() const
{
    return peerAddress().toString();
}
