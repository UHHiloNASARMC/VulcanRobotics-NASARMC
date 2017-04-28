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
        self._socket.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 128)
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
            except Exception as e:
                print(e)
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

kThrottle = talonsrx.TalonSrxProtocol.kThrottle
kPosition = talonsrx.TalonSrxProtocol.kPositionMode
kDisabled = talonsrx.TalonSrxProtocol.kDisabled

def ArmDegreesToCounts(degrees):
    return degrees * 4

def ArmCountsToDegrees(counts):
    return counts // 4

class ArmDriveToPosition:
    def __init__(self, angle, threshold):
        self._counts = ArmDegreesToCounts(angle)
        self._threshold = ArmDegreesToCounts(threshold)
        
    def update(self, ctrl):
        pot = ctrl._arm.getPotValue()

class BucketDriveToPosition:
    def __init__(self, angle):
        self._ctrl = control_loop

class ArmControlLoop:
    # Physical limits to enforce with utmost priority
    kMaxArmCountsPerSec = ArmDegreesToCounts(75)
    kMaxArmThrottleCountsPerSec = 2048

    def __init__(self, arm_talon, bucket_talon):
        self._arm = arm_talon
        self._bucket = bucket_talon
        self._lastTime = None
        self._lastArmTime = None
        self._armVelocity = None
        self._armPot = None
        self._armSetPoint = None
        self._armThrottle = None
        self._lastArmPot = None
        self._lastBucketTime = None
        self._bucketPot = None
        self._lastBucketPot = None
        self._state = 0
        self._taskList = []

    def setState(self, state):
        # 0: Idle, 1: Scooping, 2: Driving, 3: Dumping, 4: Weighting
        if state == 0:
            self._arm.setDemand(0, kDisabled)
        elif state == 1:
            
        self._state = state

    def killLoop(self):
        # Comms lost - kill motors and reset control loop
        self.setState(0)
        self._lastTime = None

    def update(self):
        # Scan present states
        self._armPot, armPotNew = self._arm.getPotValue()
        self._bucketPot, bucketPotNew = self._bucket.getPotValue()
        curTime = time.time()

        if self._lastTime is None:
            # Initialize temporal states
            self._lastTime = curTime
            self._armVelocity = 0
            self._lastArmPot = self._armPot
            self._lastBucketPot = self._bucketPot
        else:
            # Advance temporal states
            dt = curTime - self._lastTime
            self._lastTime = curTime
            maxArmThrottleDelta = kMaxArmThrottleCountsPerSec * dt
            
            if armPotNew:
                armDt = curTime - self._lastArmTime
                self._lastArmTime = curTime
                deltaAng = self._armPot - self._lastArmPot
                self._armVelocity = deltaAng / armDt
                self._lastArmPot = self._armPot

            if bucketPotNew:
                bucketDt = curTime - self._lastBucketTime
                self._lastBucketTime = curTime
                deltaAng = self._bucketPot - self._lastBucketPot
                self._bucketVelocity = deltaAng / bucketDt
                self._lastBucketPot = self._bucketPot

            # Update current task
            if len(self._taskList):
                if self._taskList[0].update(self):
                    # Task complete
                    self._taskList.pop(0)

            if False:
                # Update arm set velocity
                armError = abs(self._armSetPoint - self._armPot)
                if armError < 30:
                    armSetVelocity = 0
                elif armError < 230:
                    armSetVelocity = kMaxArmCountsPerSec * ((armError - 30) / 200)
                else:
                    armSetVelocity = kMaxArmCountsPerSec
                    
                if self._armSetPoint < self._armPot:
                    armSetVelocity *= -1.0

                # Update arm throttle
                if armSetVelocity < self._armVelocity:
                    self._armThrottle -= maxArmThrottleDelta
                elif armSetVelocity > self._armVelocity:
                    self._armThrottle += maxArmThrottleDelta
                self._arm.setDemand(self._armThrottle, kThrottle)
            else:
                # Talon-hosted control loop
                self._arm.setDemand(self._armSetPoint, kPosition)

        
            

# Setup talon interface
intf = talonsrx.TalonCANInterface()
bucket = talonsrx.TalonSrxProtocol(intf, 4)
arm = talonsrx.TalonSrxProtocol(intf, 3)
front_left = talonsrx.TalonSrxProtocol(intf, 5)
front_right = talonsrx.TalonSrxProtocol(intf, 6)
back_left = talonsrx.TalonSrxProtocol(intf, 7)
back_right = talonsrx.TalonSrxProtocol(intf, 8)

spockSocket = SpockSocket()

def killMotors():
    #print('No heartbeat')
    bucket.setDemand(0, kDisabled)
    arm.setDemand(0, kDisabled)
    front_left.setDemand(0, kDisabled)
    back_left.setDemand(0, kDisabled)
    front_right.setDemand(0, kDisabled)
    back_right.setDemand(0, kDisabled)

# Main comm loop
while True:
    if spockSocket.receiveCommandPacket(killMotors):
        #print('Left', spockSocket._leftThrottle,
        #      'Right', spockSocket._rightThrottle,
        #      'Arm', spockSocket._armAngleBias,
        #      'Bucket', spockSocket._linearActuatorBias)
        bucket.setDemand(spockSocket._linearActuatorBias // 2, kThrottle)
        arm.setDemand(spockSocket._armAngleBias // 2, kThrottle)
        front_left.setDemand(-spockSocket._leftThrottle, kThrottle)
        back_left.setDemand(-spockSocket._leftThrottle, kThrottle)
        front_right.setDemand(spockSocket._rightThrottle, kThrottle)
        back_right.setDemand(spockSocket._rightThrottle, kThrottle)
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
    spockSocket._armPot = arm.getPotValue()
    spockSocket._bucketPot = bucket.getPotValue()
    spockSocket.sendStatusPacket()
    

