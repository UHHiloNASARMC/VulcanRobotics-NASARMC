import talonsrx, time, math
from RPi import GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup(24, GPIO.OUT)
GPIO.output(24, GPIO.HIGH)

intf = talonsrx.TalonCANInterface()
TalonSrxProtocol = talonsrx.TalonSrxProtocol
proto = TalonSrxProtocol(intf, 3)
proto.setRevFeedbackSensor(1)
proto.setRampThrottle(10)

bucket_proto = TalonSrxProtocol(intf, 4)
bucket_proto.setPeakThrottle(512)
bucket_proto.setNominalThrottle(512)
bucket_proto.setP(0.03)
bucket_proto.setRevFeedbackSensor(1)
bucket_proto.setRampThrottle(10)

def PotCountsToRadians(counts):
    return -(counts - 590) * math.pi / 700

def HyperbolicFalloff(x):
    return min(1.0, max(0.0, 0.1 * (0.8 * x ** 2.0 + 0.4) * 8.0 * math.sqrt(x)))

class ArmDriveToXfer:
    def __init__(self, ctrl):
        self._ctrl = ctrl
        self._timeInXfer = 0.0
    def update(self, angle, bucket_pot):
        print('ArmDriveToXfer')
        self._ctrl._rampFactor = 1.0
        self._ctrl._setAngle = -0.5
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

class ArmControlLoop:
    def __init__(self, arm_talon, bucket_talon):
        self._arm = arm_talon
        self._bucket = bucket_talon
        self._state = 0
        self._curTime = 0.0
        self._taskList = []
        self._setAngle = -math.pi / 2.0
        self._rampFactor = 1.0

    def setState(self, state):

        # Read pot value
        cur_point = -self._arm.getPotValue()
        cur_angle = PotCountsToRadians(cur_point)

        # Read bucket pot
        bucket_pot = -self._bucket.getPotValue()

        # 0: Idle, 1: Scooping, 2: Driving, 3: Dumping, 4: Weighting
        if state == 0:
            self._arm.setDemand(0, TalonSrxProtocol.kDisabled)
            self._bucket.setDemand(0, TalonSrxProtocol.kDisabled)
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

        # Update current task
        if len(self._taskList):
            if self._taskList[0].update(cur_angle, bucket_pot):
                print('Done')
                self._taskList.pop(0)

        # Compute counter-drive throttle
        base_throttle = math.cos(cur_angle) * 240

        # Compute error throttle
        #test_angle = -1.0
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
        self._arm.setDemand(total_throttle, TalonSrxProtocol.kThrottle)
        current = self._arm.getCurrent()
        print('Counts', cur_point, 'Bucket', bucket_pot, 'Ang', cur_angle, 'Base', base_throttle, 'Error', err_throttle, 'Total', total_throttle, 'Amps', current * 0.125)

while True:
    if proto.hasData() and bucket_proto.hasData():
        break
    time.sleep(0.02)

ctrl_loop = ArmControlLoop(proto, bucket_proto)
ctrl_loop.setState(1)
while True:
    # Scan pot value
    curpoint = proto.getPotValue()
    forward_lim = proto.getForwardLimitSwitch()
    rev_lim = proto.getReverseLimitSwitch()
    throttle = proto.getAppliedThrottle()
    ctrl_loop.update()
    #print('Ang', PotCountsToRadians(-curpoint))
    #print('Value', curpoint, 'Fwd', forward_lim, 'Rev', rev_lim, 'Throttle', throttle)

    time.sleep(0.02)
