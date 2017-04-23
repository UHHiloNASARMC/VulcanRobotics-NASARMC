import socket, struct, time
from threading import Thread, Lock
import talonsrx

# Socket for all rover communication
class SpockSocket:
    def __init__(self):
        self._server = socket.socket(type=socket.SOCK_STREAM)
        self._server.bind(('0.0.0.0', 5555))
        self._server.listen(1)
        self._socket = None
        
        self._lastSendPacket = 0
        self._frontLeftCurrent = 0
        self._frontLeftBattV = 0
        self._frontLeftTemp = 0
        self._frontRightCurrent = 0
        self._frontRightBattV = 0
        self._frontRightTemp = 0
        self._rearLeftCurrent = 0
        self._rearLeftBattV = 0
        self._rearLeftTemp = 0
        self._rearRightCurrent = 0
        self._rearRightBattV = 0
        self._rearRightTemp = 0
        self._armCurrent = 0
        self._armBattV = 0
        self._armTemp = 0
        self._bucketCurrent = 0
        self._bucketBattV = 0
        self._bucketTemp = 0
        self._armPot = 0
        self._bucketPot = 0
        self._bucketState = 0

        self._lastRecvTime = None
        self._lastRecvPacket = 0
        self._leftThrottle = 0
        self._rightThrottle = 0
        self._bucketState = 0
        self._bucketOscillate = 0
        self._armAngleBias = 0
        self._linearActuatorBias = 0
        self._cameraEnables = 0

        self.reestablishClient()

    def reestablishClient(self):
        print('Waiting for client')
        self._socket, addrfrom = self._server.accept()
        self._socket.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 35)
        self._socket.settimeout(1.0)
        print('Connection from ', addrfrom)

    def sendStatusPacket(self):
        self._lastSendPacket += 1
        data = struct.pack('>8sQQQHBBHBBHBBHBBHBBHBBHHB', b'RoboStat', int(time.time()),
                           self._lastSendPacket, self._lastRecvPacket,
                           self._frontLeftCurrent, self._frontLeftBattV, self._frontLeftTemp,
                           self._frontRightCurrent, self._frontRightBattV, self._frontRightTemp,
                           self._rearLeftCurrent, self._rearLeftBattV, self._rearLeftTemp,
                           self._rearRightCurrent, self._rearRightBattV, self._rearRightTemp,
                           self._armCurrent, self._armBattV, self._armTemp,
                           self._bucketCurrent, self._bucketBattV, self._bucketTemp,
                           self._armPot, self._bucketPot, self._bucketState)
        try:
            self._socket.send(data)
        except:
            pass

    def receiveCommandPacket(self, timeoutHandler):
        data = None
        while True:
            try:
                data = self._socket.recv(35)
                if len(data) == 0:
                    raise RuntimeError('no data')
                break
            except:
                timeoutHandler()
                self.reestablishClient()
        if data and len(data) >= 35:
            magic, timestamp, packet_count = struct.unpack('>8sQQ', data[0:24])
            if magic == b'RoboComm':
                self._lastRecvTime = time.time()
                self._lastRecvPacket = packet_count
                self._leftThrottle, self._rightThrottle, self._bucketState,\
                self._bucketOscillate, self._armAngleBias, self._linearActuatorBias,\
                self._cameraEnables = struct.unpack('>hhBBhhB', data[24:35])
                return True
        return False
            

# Setup talon interface
intf = talonsrx.TalonCANInterface()
bucket = talonsrx.TalonSrxProtocol(intf, 1)
arm = talonsrx.TalonSrxProtocol(intf, 2)
front_left = talonsrx.TalonSrxProtocol(intf, 5)
front_right = talonsrx.TalonSrxProtocol(intf, 6)
back_left = talonsrx.TalonSrxProtocol(intf, 7)
back_right = talonsrx.TalonSrxProtocol(intf, 8)

spockSocket = SpockSocket()

def killMotors():
    print('No heartbeat')
    bucket.setDemand(0, talonsrx.TalonSrxProtocol.kThrottle)
    arm.setDemand(0, talonsrx.TalonSrxProtocol.kThrottle)
    front_left.setDemand(0, talonsrx.TalonSrxProtocol.kThrottle)
    back_left.setDemand(0, talonsrx.TalonSrxProtocol.kThrottle)
    front_right.setDemand(0, talonsrx.TalonSrxProtocol.kThrottle)
    back_right.setDemand(0, talonsrx.TalonSrxProtocol.kThrottle)

# Main comm loop
while True:
    if spockSocket.receiveCommandPacket(killMotors):
        print('Left', spockSocket._leftThrottle,
              'Right', spockSocket._rightThrottle,
              'Arm', spockSocket._armAngleBias,
              'Bucket', spockSocket._linearActuatorBias)
        bucket.setDemand(spockSocket._linearActuatorBias, talonsrx.TalonSrxProtocol.kThrottle)
        arm.setDemand(spockSocket._armAngleBias, talonsrx.TalonSrxProtocol.kThrottle)
        front_left.setDemand(spockSocket._leftThrottle, talonsrx.TalonSrxProtocol.kThrottle)
        back_left.setDemand(spockSocket._leftThrottle, talonsrx.TalonSrxProtocol.kThrottle)
        front_right.setDemand(spockSocket._rightThrottle, talonsrx.TalonSrxProtocol.kThrottle)
        back_right.setDemand(spockSocket._rightThrottle, talonsrx.TalonSrxProtocol.kThrottle)
    else:
        killMotors()

    spockSocket._frontLeftCurrent = front_left.getCurrent()
    spockSocket._frontLeftBattV = front_left.getBattV()
    spockSocket._frontLeftTemp = front_left.getTemp()
    spockSocket._frontRightCurrent = front_right.getCurrent()
    spockSocket._frontRightBattV = front_right.getBattV()
    spockSocket._frontRightTemp = front_right.getTemp()
    spockSocket._rearLeftCurrent = back_left.getCurrent()
    spockSocket._rearLeftBattV = back_left.getBattV()
    spockSocket._rearLeftTemp = back_left.getTemp()
    spockSocket._rearRightCurrent = back_right.getCurrent()
    spockSocket._rearRightBattV = back_right.getBattV()
    spockSocket._rearRightTemp = back_right.getTemp()
    spockSocket._armCurrent = arm.getCurrent()
    spockSocket._armBattV = arm.getBattV()
    spockSocket._armTemp = arm.getTemp()
    spockSocket._bucketCurrent = bucket.getCurrent()
    spockSocket._bucketBattV = bucket.getBattV()
    spockSocket._bucketTemp = bucket.getTemp()
    spockSocket.sendStatusPacket()
    

