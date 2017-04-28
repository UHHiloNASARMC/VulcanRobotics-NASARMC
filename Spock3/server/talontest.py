import talonsrx, time

intf = talonsrx.TalonCANInterface()
TalonSrxProtocol = talonsrx.TalonSrxProtocol
proto = TalonSrxProtocol(intf, 3)
proto.setPeakThrottle(512)
proto.setNominalThrottle(512)
proto.setP(0.03)
proto.setRevFeedbackSensor(1)
#proto.setAllowableClosedLoopError(1)
#proto.setCloseLoopRampRate(10)
proto.setRampThrottle(10)

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
    setpoint = full_span * 0.5 + start
    proto.setDemand(-setpoint, TalonSrxProtocol.kPositionMode)

    # Scan pot value
    curpoint = proto.getPotValue()
    print('Value', curpoint)

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
