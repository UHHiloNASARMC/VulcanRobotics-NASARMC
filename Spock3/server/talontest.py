import talonsrx, time
from RPi import GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup(24, GPIO.OUT)
GPIO.output(24, GPIO.HIGH)

intf = talonsrx.TalonCANInterface()
TalonSrxProtocol = talonsrx.TalonSrxProtocol
proto = TalonSrxProtocol(intf, 3)
proto.setPeakThrottle(512)
proto.setNominalThrottle(512)
proto.setP(0.1)
proto.setRevFeedbackSensor(1)
#proto.setAllowableClosedLoopError(1)
#proto.setCloseLoopRampRate(10)
proto.setRampThrottle(10)
proto.setMotionMagicAccel(1)
proto.setMotionMagicCruiseVel(1)

cycle_count = 0
setpoint = 256
tolerance = 100
start = 28
full_span = 845
while True:
    #if cycle_count == 0:
    #    print('Running to 0%')
    #    setpoint = start
    #    proto.setDemand(-setpoint, TalonSrxProtocol.kPositionMode)
    #if cycle_count == 60:
    #    print('Running to 25%')
    #    setpoint = full_span * 0.25 + start
    #    proto.setDemand(-setpoint, TalonSrxProtocol.kPositionMode)
    #if cycle_count == 120:
    #    print('Running to 50%')
    #    setpoint = full_span * 0.5 + start
    #    proto.setDemand(-setpoint, TalonSrxProtocol.kPositionMode)
    #if cycle_count == 180:
    #    print('Running to 100%')
    #    setpoint = full_span + start
    #    proto.setDemand(-setpoint, TalonSrxProtocol.kPositionMode)
    #setpoint = full_span * 0.5 + start
    #setpoint = 900
    proto.setDemand(-900, TalonSrxProtocol.kMotionMagic)

    # Scan pot value
    curpoint = proto.getPotValue()
    forward_lim = proto.getForwardLimitSwitch()
    rev_lim = proto.getReverseLimitSwitch()
    throttle = proto.getAppliedThrottle()
    print('Value', curpoint, 'Fwd', forward_lim, 'Rev', rev_lim, 'Throttle', throttle)

    #if curpoint < setpoint - tolerance:
        # Drive forward
        #print('Forward')
        #proto.setDemand(512, TalonSrxProtocol.kThrottle)
    #elif curpoint > setpoint + tolerance:
        # Drive backward
        #print('Backward')
        #proto.setDemand(-512, TalonSrxProtocol.kThrottle)
    #else:
        # Stop
        #print('Stop')
        #proto.setDemand(0, TalonSrxProtocol.kThrottle)

    time.sleep(0.1)
    cycle_count += 1
    if cycle_count == 240:
        cycle_count = 0
