import socket, struct, time, math
from threading import Thread, Lock
import talonsrx
TalonSrxProtocol = talonsrx.TalonSrxProtocol

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
        self._sensedState = 0
        self._limitSwitches = 0

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
        self._socket.settimeout(5.0)
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
                           -self._armPot, -self._bucketPot, self._sensedState)
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

def PotCountsToRadians(counts):
    return -(counts - 590) * math.pi / 700

##################
## Bucket Tasks ##
##################

class ArmDriveToXfer:
    def __init__(self, ctrl):
        self._ctrl = ctrl
        self._timeInXfer = 0.0
    def update(self, angle, bucket_pot):
        print('ArmDriveToXfer')
        self._ctrl._rampFactor = 1.0
        self._ctrl._setAngle = -0.5
        self._ctrl._bucket.setDemand(-900, TalonSrxProtocol.kPositionMode)
        if abs(angle + 0.5) < 0.1:
            self._timeInXfer += 0.01
            return self._timeInXfer > 0.5
        else:
            self._timeInXfer = 0.0
        return False

class ArmDriveToScoop:
    def __init__(self, ctrl):
        self._ctrl = ctrl
    def update(self, angle, bucket_pot):
        print('ArmDriveToScoop')
        self._ctrl._rampFactor = 1.0
        self._ctrl._setAngle = -math.pi / 2.0
        return angle < -1.3

class ArmDriveToDump:
    def __init__(self, ctrl):
        self._ctrl = ctrl
    def update(self, angle, bucket_pot):
        print('ArmDriveToDump')
        self._ctrl._rampFactor = 1.0
        self._ctrl._setAngle = 2.5
        self._ctrl._bucket.setDemand(-450, TalonSrxProtocol.kPositionMode)
        return angle > 2.45

class ArmRampDownThrottle:
    def __init__(self, ctrl):
        self._ctrl = ctrl
        self._throttle = 1.0
    def update(self, angle, bucket_pot):
        print('ArmRampDownThrottle')
        self._throttle = max(0.0, self._throttle - 0.01)
        self._ctrl._rampFactor = self._throttle
        return self._throttle == 0.0

class RetractBucket:
    def __init__(self, ctrl):
        self._ctrl = ctrl
    def update(self, angle, bucket_pot):
        print('RetractBucket')
        self._ctrl._bucket.setDemand(-900, TalonSrxProtocol.kPositionMode)
        return bucket_pot > 860

class BucketToRest:
    def __init__(self, ctrl):
        self._ctrl = ctrl
    def update(self, angle, bucket_pot):
        print('BucketToRest')
        self._ctrl._bucket.setDemand(-300, TalonSrxProtocol.kPositionMode)
        return abs(bucket_pot - 300) < 10

class BucketToScoop:
    def __init__(self, ctrl):
        self._ctrl = ctrl
    def update(self, angle, bucket_pot):
        print('BucketToScoop')
        self._ctrl._bucket.setDemand(-450, TalonSrxProtocol.kPositionMode)
        return abs(bucket_pot - 450) < 10

##################
## Control Loop ##
##################

class ArmControlLoop:
    def __init__(self, arm_talon, bucket_talon):
        self._arm = arm_talon
        self._bucket = bucket_talon
        self._state = 0
        self._curTime = 0.0
        self._taskList = []
        self._setAngle = -math.pi / 2.0
        self._rampFactor = 1.0
        self._sensedState = 0
        self._actingState = 0
        self._throttleBias = 0

    def setState(self, state):
        if state == self._actingState:
            return
        self._actingState = state
        self._taskList.clear()

        # Read pot value
        cur_point = -self._arm.getPotValue()
        cur_angle = PotCountsToRadians(cur_point)

        # Read bucket pot
        bucket_pot = -self._bucket.getPotValue()

        # 0: Idle, 1: Scooping, 2: Driving, 3: Dumping, 4: Weighting
        if state == 0:
            self._arm.setDemand(0, TalonSrxProtocol.kThrottle)
            self._bucket.setDemand(0, TalonSrxProtocol.kThrottle)
        elif state == 1:
            if cur_angle > -0.7:
                self._taskList.append(ArmDriveToXfer(self))
                self._taskList.append(RetractBucket(self))
            self._taskList.append(ArmDriveToScoop(self))
            self._taskList.append(ArmRampDownThrottle(self))
            self._taskList.append(BucketToScoop(self))
        elif state == 2:
            if cur_angle <= -0.7:
                self._taskList.append(ArmDriveToScoop(self))
                self._taskList.append(RetractBucket(self))
            self._taskList.append(ArmDriveToXfer(self))
            self._taskList.append(BucketToRest(self))
            self._taskList.append(ArmRampDownThrottle(self))
        elif state == 3:
            if cur_angle <= -0.7:
                self._taskList.append(ArmDriveToScoop(self))
                self._taskList.append(RetractBucket(self))
            self._taskList.append(ArmDriveToXfer(self))
            self._taskList.append(ArmDriveToDump(self))
            self._taskList.append(ArmRampDownThrottle(self))
            self._taskList.append(RetractBucket(self))


    def update(self):
        self._curTime += 0.02

        # Read pot value
        cur_point = -self._arm.getPotValue()
        cur_angle = PotCountsToRadians(cur_point)

        # Read bucket pot
        bucket_pot = -self._bucket.getPotValue()

        # Sense current state
        if cur_angle <= -0.7:
            self._sensedState = 1
        elif cur_angle > 2.45:
            self._sensedState = 3
        else:
            self._sensedState = 2

        # Update current task
        if len(self._taskList):
            if self._taskList[0].update(cur_angle, bucket_pot):
                print('Done')
                self._taskList.pop(0)

        # Compute counter-drive throttle
        base_throttle = math.cos(cur_angle) * 240

        # Compute error throttle
        err_angle = self._setAngle - cur_angle
        err_throttle = err_angle * 300
        if err_throttle < -200:
            err_throttle = -200
        elif err_throttle > 200:
            err_throttle = 200

        # Steady drop
        if cur_angle < 0.0 and cur_angle > -1.4 and err_angle < -0.2:
            base_throttle += 75

        # Drive motor
        total_throttle = (base_throttle + err_throttle) * self._rampFactor
        total_throttle += self._throttleBias
        self._arm.setDemand(total_throttle, TalonSrxProtocol.kThrottle)
        current = self._arm.getCurrent()
        print('Counts', cur_point, 'Bucket', bucket_pot, 'Ang', cur_angle, 'Base',
              base_throttle, 'Error', err_throttle, 'Total', total_throttle, 'Amps', current * 0.125)


# Setup talon interface
intf = talonsrx.TalonCANInterface()
bucket = talonsrx.TalonSrxProtocol(intf, 4)
arm = talonsrx.TalonSrxProtocol(intf, 3)
front_left = talonsrx.TalonSrxProtocol(intf, 5)
front_right = talonsrx.TalonSrxProtocol(intf, 6)
back_left = talonsrx.TalonSrxProtocol(intf, 7)
back_right = talonsrx.TalonSrxProtocol(intf, 8)

arm.setRevFeedbackSensor(1)
arm.setRampThrottle(10)

bucket.setPeakThrottle(512)
bucket.setNominalThrottle(512)
bucket.setP(0.03)
bucket.setRevFeedbackSensor(1)
bucket.setRampThrottle(10)

# Network socket
spockSocket = SpockSocket()

# Wait for initial data from talons
while True:
    if bucket.hasData() and arm.hasData():
        break
    time.sleep(0.02)

# Arm control loop
ctrl_loop = ArmControlLoop(arm, bucket)

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
        ctrl_loop.setState(spockSocket._bucketState)
        front_left.setDemand(-spockSocket._leftThrottle, kThrottle)
        back_left.setDemand(-spockSocket._leftThrottle, kThrottle)
        front_right.setDemand(spockSocket._rightThrottle, kThrottle)
        back_right.setDemand(spockSocket._rightThrottle, kThrottle)
        ctrl_loop._throttleBias = spockSocket._armAngleBias // 2
        ctrl_loop.update()

        if spockSocket._linearActuatorBias:
            bucket.setDemand(spockSocket._linearActuatorBias // 2, kThrottle)
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
    spockSocket._limitSwitches = arm.getForwardLimitSwitch() | \
                                 (arm.getReverseLimitSwitch() << 1)
    spockSocket._sensedState = ctrl_loop._sensedState
    spockSocket.sendStatusPacket()


