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
        self._t = Thread(target=self.commTask)
        self._running = True
        self._l = Lock()

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
        self._timedOut = False

        self.reestablishClient()
        self._t.start()

    def __del__(self):
        self._running = False
        self._t.join()

    def reestablishClient(self):
        print('Waiting for client')
        self._socket, addrfrom = self._server.accept()
        self._socket.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 64)
        self._socket.settimeout(5.0)
        print('Connection from ', addrfrom)
        self._timedOut = False

    def sendStatusPacket(self):
        self._l.acquire()
        self._lastSendPacket += 1
        data = struct.pack('>8sQQQHBBHBBHBBHBBHBBHBBhhBB', b'RoboStat', int(time.time()),
                           self._lastSendPacket, self._lastRecvPacket,
                           self._frontLeftCurrent, self._frontLeftBattV, self._frontLeftTemp,
                           self._frontRightCurrent, self._frontRightBattV, self._frontRightTemp,
                           self._rearLeftCurrent, self._rearLeftBattV, self._rearLeftTemp,
                           self._rearRightCurrent, self._rearRightBattV, self._rearRightTemp,
                           self._armCurrent, self._armBattV, self._armTemp,
                           self._bucketCurrent, self._bucketBattV, self._bucketTemp,
                           self._armPot, -self._bucketPot, self._sensedState, self._limitSwitches)
        data += b'\0\0'
        self._l.release()
        try:
            self._socket.send(data)
            #print('Did send', self._lastSendPacket)
        except Exception as e:
            print('Send', e)

    def receiveCommandPacket(self):
        data = None
        while True:
            try:
                data = self._socket.recv(64)
                if len(data) == 0:
                    raise RuntimeError('no data')
                break
            except Exception as e:
                print('Recv', e)
                self._timedOut = True
                self.reestablishClient()
        if data and len(data) >= 64:
            magic, timestamp, packet_count = struct.unpack('>8sQQ', data[0:24])
            if magic == b'RoboComm':
                self._l.acquire()
                self._lastRecvTime = time.time()
                self._lastRecvPacket = packet_count
                #print('Did recv', packet_count)
                self._leftThrottle, self._rightThrottle, self._bucketState,\
                self._bucketOscillate, self._armAngleBias, self._linearActuatorBias,\
                self._cameraEnables = struct.unpack('>hhBBhhB', data[24:35])
                self._l.release()
                return True
        return False

    def commTask(self):
        while self._running:
            if self.receiveCommandPacket():
                self.sendStatusPacket()

kThrottle = talonsrx.TalonSrxProtocol.kThrottle
kPosition = talonsrx.TalonSrxProtocol.kPositionMode
kDisabled = talonsrx.TalonSrxProtocol.kDisabled

def PotCountsToRadians(counts):
    return (-counts + 590) * math.pi / 700

def EncoderCountsToRadians(counts):
    return 0.0127 * counts
kEncoderLow = -96
kEncoderHigh = 184

##################
## Bucket Tasks ##
##################

class ArmDriveToXfer:
    def __init__(self, ctrl):
        self._ctrl = ctrl
        self._timeInXfer = 0.0
    def update(self, angle, bucket_pot, dt, half_second_angle_delta):
        print('ArmDriveToXfer')
        self._ctrl._totalThrottleGain = 1.0
        self._ctrl._setAngle = -0.5
        self._ctrl._bucket.setDemand(-814, TalonSrxProtocol.kPositionMode)
        if abs(angle + 0.5) < 0.1 and dt is not None:
            self._timeInXfer += dt
            return self._timeInXfer > 0.5
        else:
            self._timeInXfer = 0.0
            if half_second_angle_delta and abs(half_second_angle_delta) < 0.1:
                self._ctrl._baseThrottleGain += 0.1
        return False

class ArmDriveToScoop:
    def __init__(self, ctrl):
        self._ctrl = ctrl
    def update(self, angle, bucket_pot, dt, half_second_angle_delta):
        print('ArmDriveToScoop')
        self._ctrl._totalThrottleGain = 1.0
        self._ctrl._setAngle = -math.pi / 2.0
        self._ctrl._baseThrottleGain = 1.0
        return angle < -1.25

class ArmDriveToDump:
    def __init__(self, ctrl):
        self._ctrl = ctrl
    def update(self, angle, bucket_pot, dt, half_second_angle_delta):
        print('ArmDriveToDump')
        self._ctrl._totalThrottleGain = 1.0
        self._ctrl._setAngle = 2.5
        self._ctrl._bucket.setDemand(-450, TalonSrxProtocol.kPositionMode)
        if angle > 2.45:
            self._ctrl._baseThrottleGain = 1.0
            return True
        return False

class ArmRampDownThrottle:
    def __init__(self, ctrl):
        self._ctrl = ctrl
        self._throttle = 1.0
    def update(self, angle, bucket_pot, dt, half_second_angle_delta):
        print('ArmRampDownThrottle')
        self._throttle = max(0.0, self._throttle - 0.01)
        self._ctrl._totalThrottleGain = self._throttle
        return self._throttle == 0.0

class RetractBucket:
    def __init__(self, ctrl):
        self._ctrl = ctrl
    def update(self, angle, bucket_pot, dt, half_second_angle_delta):
        print('RetractBucket')
        self._ctrl._bucket.setDemand(-814, TalonSrxProtocol.kPositionMode)
        return bucket_pot > 810

class BucketToRest:
    def __init__(self, ctrl):
        self._ctrl = ctrl
    def update(self, angle, bucket_pot, dt, half_second_angle_delta):
        print('BucketToRest')
        self._ctrl._bucket.setDemand(-280, TalonSrxProtocol.kPositionMode)
        return abs(bucket_pot - 280) < 10

class BucketToScoop:
    def __init__(self, ctrl):
        self._ctrl = ctrl
    def update(self, angle, bucket_pot, dt, half_second_angle_delta):
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
        self._taskList = []
        self._setAngle = -math.pi / 2.0
        self._totalThrottleGain = 1.0
        self._baseThrottleGain = 1.0
        self._sensedState = 0
        self._actingState = 0
        self._throttleBias = 0
        self._linearActuatorBias = 0
        self._lastLinearActuatorBias = 0
        self._lastTime = None
        self._lastHalfSecondAngle = None
        self._oscillateTarget = 350

    def setState(self, state):
        if state == self._actingState:
            return
        self._actingState = state
        self._taskList.clear()

        # Read pot value
        cur_point = self._arm.getSensorPosition()
        cur_angle = EncoderCountsToRadians(cur_point)

        # Read bucket pot
        bucket_pot = -self._bucket.getSensorPosition()

        # 0: Idle, 1: Scooping, 2: Driving, 3: Dumping, 4: Weighting
        if state == 0:
            self._totalThrottleGain = 0.0
            self._baseThrottleGain = 1.0
            self._arm.setDemand(0, TalonSrxProtocol.kThrottle)
            self._bucket.setDemand(0, TalonSrxProtocol.kThrottle)
        elif state == 1:
            if cur_angle > -0.75:
                self._taskList.append(ArmDriveToXfer(self))
                self._taskList.append(RetractBucket(self))
            self._taskList.append(ArmDriveToScoop(self))
            self._taskList.append(ArmRampDownThrottle(self))
            self._taskList.append(BucketToScoop(self))
        elif state == 2:
            if cur_angle <= -0.75:
                self._taskList.append(ArmDriveToScoop(self))
                self._taskList.append(RetractBucket(self))
            self._taskList.append(ArmDriveToXfer(self))
            self._taskList.append(BucketToRest(self))
            self._taskList.append(ArmRampDownThrottle(self))
        elif state == 3:
            if cur_angle <= -0.75:
                self._taskList.append(ArmDriveToScoop(self))
                self._taskList.append(RetractBucket(self))
            self._taskList.append(ArmDriveToXfer(self))
            self._taskList.append(ArmDriveToDump(self))
            self._taskList.append(ArmRampDownThrottle(self))
            self._taskList.append(RetractBucket(self))


    # Main arm control loop
    def update(self):
        dt = None
        cur_time = time.time()
        new_half_second = False
        if self._lastTime is not None:
            dt = cur_time - self._lastTime
            new_half_second = math.floor(self._lastTime * 2.0) < math.floor(cur_time * 2.0)
        self._lastTime = cur_time

        # Reset limit angle
        if not self._arm.getReverseLimitSwitch():
            #print('LOW')
            self._arm.setSensorPosition(kEncoderLow)
        elif not self._arm.getForwardLimitSwitch():
            #print('HIGH')
            self._arm.setSensorPosition(kEncoderHigh)

        # Read pot value
        cur_point = self._arm.getSensorPosition()
        #print(cur_point)
        cur_angle = EncoderCountsToRadians(cur_point)
        half_second_angle_delta = None
        if new_half_second:
            if self._lastHalfSecondAngle is not None:
                half_second_angle_delta = cur_angle - self._lastHalfSecondAngle
            self._lastHalfSecondAngle = cur_angle

        # Read bucket pot
        bucket_pot = -self._bucket.getSensorPosition()

        # Sense current state
        if cur_angle <= -0.75:
            self._sensedState = 1
        elif cur_angle > 2.45:
            self._sensedState = 3
        else:
            self._sensedState = 2

        # Update current task
        if len(self._taskList):
            if self._taskList[0].update(cur_angle, bucket_pot, dt, half_second_angle_delta):
                print('Done')
                self._taskList.pop(0)

        # Compute counter-drive throttle
        base_throttle = math.cos(cur_angle) * 240 * self._baseThrottleGain

        # Compute error throttle
        err_angle = self._setAngle - cur_angle
        err_throttle = err_angle * 300
        if err_throttle < -200:
            err_throttle = -200
        elif err_throttle > 200:
            err_throttle = 200

        # Steady bucket drop
        if cur_angle < 0.0 and cur_angle > -1.4 and err_angle < -0.2:
            base_throttle += 75

        # Drive motor
        #total_throttle = (base_throttle + err_throttle) * self._totalThrottleGain
        total_throttle = self._throttleBias
        self._arm.setDemand(total_throttle, TalonSrxProtocol.kThrottle)
        current = self._arm.getCurrent()
        #print('Base', base_throttle, 'Error', err_throttle, 'Throttle', total_throttle)
        #print('Counts', cur_point, 'Bucket', bucket_pot, 'Ang', cur_angle, 'Base',
        #      base_throttle, 'Error', err_throttle, 'Total', total_throttle, 'Amps', current * 0.125)

        # Linear actuator bias
        if self._linearActuatorBias > 0:
            #self._lastLinearActuatorBias = self._linearActuatorBias
            #print('SET', self._linearActuatorBias)
            if bucket_pot > 350:
                self._oscillateTarget = 150
            elif bucket_pot < 150:
                self._oscillateTarget = 350
            if bucket_pot < self._oscillateTarget:
                print('SET', -256)
                self._bucket.setDemand(-256, TalonSrxProtocol.kThrottle)
            else:
                print('SET', 256)
                self._bucket.setDemand(256, TalonSrxProtocol.kThrottle)
        elif self._linearActuatorBias < 0:
            self._bucket.setDemand(self._linearActuatorBias, TalonSrxProtocol.kThrottle)
        else:
            self._bucket.setDemand(0, TalonSrxProtocol.kThrottle)

# Setup talon interface
intf = talonsrx.TalonCANInterface('can1')
intf_bucket = talonsrx.TalonCANInterface('can0')
bucket = talonsrx.TalonSrxProtocol(intf_bucket, 2)
arm = talonsrx.TalonSrxProtocol(intf, 1)
front_left = talonsrx.TalonSrxProtocol(intf, 5)
front_right = talonsrx.TalonSrxProtocol(intf, 6)
back_left = talonsrx.TalonSrxProtocol(intf, 7)
back_right = talonsrx.TalonSrxProtocol(intf, 8)

arm.setRevFeedbackSensor(1)
arm.setRampThrottle(10)
arm.setFeedbackDevice(TalonSrxProtocol.kFeedbackDevice_QuadEncoder)

bucket.setPeakThrottle(512)
bucket.setNominalThrottle(512)
bucket.setP(0.03)
bucket.setRevFeedbackSensor(1)
bucket.setRampThrottle(10)

# Network socket
spock_socket = SpockSocket()

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
    spock_socket._l.acquire()
    
    if spock_socket._timedOut:
        killMotors()
    else:
        #print('Left', spock_socket._leftThrottle,
        #      'Right', spock_socket._rightThrottle,
        #      'Arm', spock_socket._armAngleBias,
        #      'Bucket', spock_socket._linearActuatorBias)
        ctrl_loop.setState(spock_socket._bucketState)
        front_left.setDemand(-spock_socket._leftThrottle, kThrottle)
        back_left.setDemand(-spock_socket._leftThrottle, kThrottle)
        front_right.setDemand(spock_socket._rightThrottle, kThrottle)
        back_right.setDemand(spock_socket._rightThrottle, kThrottle)
        ctrl_loop._throttleBias = spock_socket._armAngleBias * 3 // 4
        ctrl_loop._linearActuatorBias = spock_socket._linearActuatorBias // 2
        ctrl_loop.update()

        spock_socket._frontLeftCurrent = front_left.getCurrent()
        spock_socket._frontLeftBattV = front_left.getBattV()
        spock_socket._frontLeftTemp = front_left.getTemp()
        spock_socket._frontRightCurrent = front_right.getCurrent()
        spock_socket._frontRightBattV = front_right.getBattV()
        spock_socket._frontRightTemp = front_right.getTemp()
        spock_socket._rearLeftCurrent = back_left.getCurrent()
        spock_socket._rearLeftBattV = back_left.getBattV()
        spock_socket._rearLeftTemp = back_left.getTemp()
        spock_socket._rearRightCurrent = back_right.getCurrent()
        spock_socket._rearRightBattV = back_right.getBattV()
        spock_socket._rearRightTemp = back_right.getTemp()
        spock_socket._armCurrent = arm.getCurrent()
        spock_socket._armBattV = arm.getBattV()
        spock_socket._armTemp = arm.getTemp()
        spock_socket._bucketCurrent = bucket.getCurrent()
        spock_socket._bucketBattV = bucket.getBattV()
        spock_socket._bucketTemp = bucket.getTemp()
        spock_socket._armPot = arm.getSensorPosition()
        spock_socket._bucketPot = bucket.getSensorPosition()
        spock_socket._limitSwitches = (not arm.getForwardLimitSwitch()) | \
                                     ((not arm.getReverseLimitSwitch()) << 1)
        spock_socket._sensedState = ctrl_loop._sensedState
    
    spock_socket._l.release()
    time.sleep(0.02)

