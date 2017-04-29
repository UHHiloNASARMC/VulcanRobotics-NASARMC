import struct, can, queue, time
from threading import Thread, Lock

# 24-bit 2's compliment sign extension
def sign_extend_24(value):
    sign_bit = 1 << 23
    return (value & (sign_bit - 1)) - (value & sign_bit)

# 11-bit 2's compliment sign extension
def sign_extend_11(value):
    sign_bit = 1 << 10
    return (value & (sign_bit - 1)) - (value & sign_bit)

# Talon CAN interface class
class TalonCANInterface:
    def __init__(self):
        self._bus = can.interface.Bus(bustype='socketcan_native', channel='can0')
        self._t = Thread(target=self.canCommTask)
        self._running = True
        self._l = Lock()
        self._status1Bufs = {}
        self._status2Bufs = {}
        self._status3Bufs = {}
        self._status4Bufs = {}
        self._status5Bufs = {}
        self._status6Bufs = {}
        self._status9Bufs = {}
        self._status10Bufs = {}
        self._control1Msgs = {}
        self._control5Msgs = {}
        self._paramSetMsgs = []
        self._t.start()

    def __del__(self):
        self._running = False
        self._t.join()

    # CAN communication thread
    def canCommTask(self):
        cycle_counter = 0
        while self._running:
            self._l.acquire()

            # Receive messages
            message = self._bus.recv(timeout=0)
            while message:
                #print(message)
                deviceId = message.arbitration_id & 0x3f
                msgType = message.arbitration_id & ~0x3f

                # STATUS_1
                if msgType == 0x02041400:
                    self._status1Bufs[deviceId] = message.data
                # STATUS_2
                elif msgType == 0x02041440:
                    self._status2Bufs[deviceId] = message.data
                # STATUS_4
                elif msgType == 0x020414C0:
                    self._status4Bufs[deviceId] = message.data

                message = self._bus.recv(timeout=0)

            # Param set messages
            for msg in self._paramSetMsgs:
                self._bus.send(msg)
                #print('PARAMSET', msg)
            self._paramSetMsgs.clear()
            # 50ms CONTROL_1 update
            if not cycle_counter % 5:
                for msg in self._control1Msgs:
                    self._bus.send(self._control1Msgs[msg])
                    #print('CTRL1', self._control1Msgs[msg])
            # 10ms CONTROL_5 update
            for msg in self._control5Msgs:
                self._bus.send(self._control5Msgs[msg])
                #print('CTRL5', self._control5Msgs[msg])

            self._l.release()

            time.sleep(0.01)
            cycle_counter += 1

    # Set enable register
    def sendControl1(self, deviceNo, value):
        msg = can.Message(arbitration_id=0x02040000 | deviceNo,
                          data=struct.pack('<H', value), extended_id=True)
        self._l.acquire()
        self._control1Msgs[deviceNo] = msg
        self._l.release()
        #print('Control 1', msg)

    # Set primary control register
    def sendControl5(self, deviceNo, value):
        msg = can.Message(arbitration_id=0x02040100 | deviceNo,
                          data=struct.pack('<Q', value), extended_id=True)
        self._l.acquire()
        self._control5Msgs[deviceNo] = msg
        self._l.release()
        #print('Control 5', msg)

    # Set param register
    def sendParamSet(self, deviceNo, data):
        print('PSET', data)
        msg = can.Message(arbitration_id=0x02041880 | deviceNo,
                  data=struct.pack('<Q', data), extended_id=True)
        self._l.acquire()
        self._paramSetMsgs.append(msg)
        self._l.release()

    # Get faults register
    def getStatus1(self, deviceNo):
        self._l.acquire()
        ret = None
        if deviceNo in self._status1Bufs:
            ret = struct.unpack('<Q', self._status1Bufs[deviceNo])[0]
        self._l.release()
        return ret

    # Get feedback register
    def getStatus2(self, deviceNo):
        self._l.acquire()
        ret = None
        if deviceNo in self._status2Bufs:
            ret = struct.unpack('<Q', self._status2Bufs[deviceNo])[0]
        self._l.release()
        return ret

        # Get analog in register
    def getStatus4(self, deviceNo):
        self._l.acquire()
        ret = None
        if deviceNo in self._status4Bufs:
            ret = struct.unpack('<Q', self._status4Bufs[deviceNo])[0]
        self._l.release()
        return ret

# Talon communication protocol interface class
class TalonSrxProtocol:

    # Control mode enum for setDemand() and setControlMode()
    kThrottle = 0
    kFollowerMode = 5
    kVoltageMode = 4
    kPositionMode = 1
    kSpeedMode = 2
    kCurrentMode = 3
    kMotionProfileMode = 6
    kMotionMagic = 7
    kDisabled = 15

    # Limit switch override enum for setOverrideLimitSwitchEn()
    kLimitSwitchOverride_UseDefaultsFromFlash = 1
    kLimitSwitchOverride_DisableFwd_DisableRev = 4
    kLimitSwitchOverride_DisableFwd_EnableRev = 5
    kLimitSwitchOverride_EnableFwd_DisableRev = 6
    kLimitSwitchOverride_EnableFwd_EnableRev = 7

    # Feedback device enum for setFeedbackDevice()
    kFeedbackDevice_QuadEncoder = 0
    kFeedbackDevice_AnalogPot = 2
    kFeedbackDevice_AnalogEncoder = 3
    kFeedbackDevice_EncRising = 4
    kFeedbackDevice_EncFalling = 5
    kFeedbackDevice_CtreMagEncoder_Relative = 6
    kFeedbackDevice_CtreMagEncoder_Absolute = 7
    kFeedbackDevice_PulseWidth = 8

    # Brake override mode enum for setOverrideBrakeType()
    kBrakeOverride_UseDefaultsFromFlash = 0
    kBrakeOverride_OverrideCoast = 1
    kBrakeOverride_OverrideBrake = 2

    # Parameters for talon settings
    eProfileParamSlot0_P = 1
    eProfileParamSlot0_I = 2
    eProfileParamSlot0_D = 3
    eProfileParamSlot0_CloseLoopRampRate = 6
    ePeakPosOutput = 104
    eNominalPosOutput = 105
    ePeakNegOutput = 106
    eNominalNegOutput = 107
    eProfileParamSlot0_AllowableClosedLoopErr = 111
    eProfileParamVcompRate = 116
    eMotMag_Accel = 122
    eMotMag_VelCruise = 123

    # Constructor initializes Talon to idle
    def __init__(self, canIntf, deviceNo):
        self._cache = 0
        self._canIntf = canIntf
        self._deviceNo = deviceNo
        canIntf.sendControl1(deviceNo, 0)
        canIntf.sendControl5(deviceNo, 0)
        self.setOverrideLimitSwitchEn(TalonSrxProtocol.kLimitSwitchOverride_UseDefaultsFromFlash)
        self.setFeedbackDevice(TalonSrxProtocol.kFeedbackDevice_AnalogPot)
        self.setCloseLoopRampRate(3.0)

    # Send primary control packet to Talon
    def sendControl5(self):
        self._canIntf.sendControl5(self._deviceNo, self._cache)

    # Send param set to Talon
    def sendParamSet(self, enum, value):
        self._canIntf.sendParamSet(self._deviceNo, ((int(value) & 0xffffffff) << 8) | (enum & 0xff))

    # Set primary drive value and mode
    def setDemand(self, value, ctrlMode):
        value = int(value)
        self._cache &= ~(0xff << 16)
        self._cache &= ~(0xff << 24)
        self._cache &= ~(0xff << 32)
        self._cache |= (value & 0xff0000)
        self._cache |= (value & 0x00ff00) << 16
        self._cache |= (value & 0x0000ff) << 32
        self._cache &= ~(0xf << 52)
        self._cache |= (ctrlMode & 0xf) << 52
        self.sendControl5()

    # Set control mode itself
    def setControlMode(self, ctrlMode):
        self._cache &= ~(0xf << 52)
        self._cache |= (ctrlMode & 0xf) << 52
        self.sendControl5()

    # Set limit switch override mode
    def setOverrideLimitSwitchEn(self, mode):
        self._cache &= ~(0x7 << 45)
        self._cache |= (mode & 0x7) << 45
        self.sendControl5()

    # Set feedback device for closed-loop operation
    def setFeedbackDevice(self, device):
        self._cache &= ~(0xf << 41)
        self._cache |= (device & 0xf) << 41
        self.sendControl5()

    # Invert motor direction for closed-loop operation
    def setRevMotDuringCloseLoopEn(self, enable):
        self._cache &= ~(0x1 << 49)
        self._cache |= (enable & 0x1) << 49
        self.sendControl5()

    # Set override brake type
    def setOverrideBrakeType(self, brakeType):
        self._cache &= ~(0x3 << 50)
        self._cache |= (brakeType & 0x3) << 50
        self.sendControl5()

    # Select PID profile slot 0/1
    def setProfileSlotSelect(self, slot):
        self._cache &= ~(0x1 << 40)
        self._cache |= (slot & 0x1) << 40
        self.sendControl5()

    # Set ramp throttle
    def setRampThrottle(self, rampThrottle):
        self._cache &= ~(0xff << 56)
        self._cache |= (rampThrottle & 0xff) << 56
        self.sendControl5()

    # Invert sensor feedback direction
    def setRevFeedbackSensor(self, reverse):
        self._cache &= ~(0x1 << 48)
        self._cache |= (reverse & 0x1) << 48
        self.sendControl5()

    # Set close loop ramp rate (V/s)
    def setCloseLoopRampRate(self, rampRate):
        self.sendParamSet(TalonSrxProtocol.eProfileParamSlot0_CloseLoopRampRate, int(rampRate * 1023.0 / 12.0 / 1000.0))

    # Set P component of control loop
    def setP(self, p):
        self.sendParamSet(TalonSrxProtocol.eProfileParamSlot0_P, float(0x400000) * p)

    # Set I component of control loop
    def setI(self, i):
        self.sendParamSet(TalonSrxProtocol.eProfileParamSlot0_I, float(0x400000) * i)

    # Set I component of control loop
    def setD(self, d):
        self.sendParamSet(TalonSrxProtocol.eProfileParamSlot0_D, float(0x400000) * d)

    # Set peak throttle voltage [0,1023]
    def setPeakThrottle(self, throttle):
        self.sendParamSet(TalonSrxProtocol.ePeakPosOutput, throttle)
        self.sendParamSet(TalonSrxProtocol.ePeakNegOutput, -throttle)

    # Set peak throttle voltage [0,1023]
    def setNominalThrottle(self, throttle):
        self.sendParamSet(TalonSrxProtocol.eNominalPosOutput, throttle)
        self.sendParamSet(TalonSrxProtocol.eNominalNegOutput, -throttle)

    # Set allowable closed loop error [0,1023]
    def setAllowableClosedLoopError(self, error):
        self.sendParamSet(TalonSrxProtocol.eProfileParamSlot0_AllowableClosedLoopErr, error)

    # Set vcomp rate [1,255]
    def setVCompRate(self, rate):
        self.sendParamSet(TalonSrxProtocol.eProfileParamVcompRate, rate)

    # Set motion magic acceleration
    def setMotionMagicAccel(self, accel):
        self.sendParamSet(TalonSrxProtocol.eMotMag_Accel, accel)

    # Set motion magic cruise velocity
    def setMotionMagicCruiseVel(self, vel):
        self.sendParamSet(TalonSrxProtocol.eMotMag_VelCruise, vel)

    # Read forward limit switch
    def getForwardLimitSwitch(self):
        status1 = self._canIntf.getStatus1(self._deviceNo)
        if status1 is None:
            return False
        return bool((status1 >> 31) & 0x1)

    # Read reverse limit switch
    def getReverseLimitSwitch(self):
        status1 = self._canIntf.getStatus1(self._deviceNo)
        if status1 is None:
            return False
        return bool((status1 >> 30) & 0x1)

    # Read current potentiometer value [0,1023]
    def getPotValue(self):
        status2 = self._canIntf.getStatus2(self._deviceNo)
        if status2 is None:
            return 0
        return sign_extend_24(((status2 & 0xff) << 16) | (status2 & 0xff00) | ((status2 & 0xff0000) >> 16))

    # Read current consumption
    def getCurrent(self):
        status2 = self._canIntf.getStatus2(self._deviceNo)
        if status2 is None:
            return 0
        high = (status2 >> 40) & 0xff
        low = (status2 >> 48) & 0xc0
        return ((high << 8) | low) >> 6

    # Read applied throttle
    def getAppliedThrottle(self):
        status1 = self._canIntf.getStatus1(self._deviceNo)
        if status1 is None:
            return 0
        high = (status1 >> 24) & 0x7
        low = (status1 >> 32) & 0xff
        return sign_extend_11((high << 8) | low)

    # Read battery voltage
    def getBattV(self):
        status4 = self._canIntf.getStatus4(self._deviceNo)
        if status4 is None:
            return 0
        return (status4 >> 48) & 0xff

    # Read temperature
    def getTemp(self):
        status4 = self._canIntf.getStatus4(self._deviceNo)
        if status4 is None:
            return 0
        return (status4 >> 40) & 0xff
         
