import talonsrx, time

intf = talonsrx.TalonCANInterface()
TalonSrxProtocol = talonsrx.TalonSrxProtocol
proto = TalonSrxProtocol(intf, 4)

cycle_count = 0
setpoint = 256
print('Running to 25%')
tolerance = 100
full_value = 890
while True:
    if cycle_count == 0:
        print('Running to 0%')
        setpoint = 0
    if cycle_count == 60:
        print('Running to 25%')
        setpoint = full_value * 0.25
    if cycle_count == 120:
        print('Running to 50%')
        setpoint = full_value * 0.5
    if cycle_count == 180:
        print('Running to 100%')
        setpoint = full_value

    # Scan pot value
    curpoint = proto.getPotValue()
    print('Value', curpoint)

    if curpoint < setpoint - tolerance:
        # Drive forward
        #print('Forward')
        proto.setDemand(512, TalonSrxProtocol.kThrottle)
    elif curpoint > setpoint + tolerance:
        # Drive backward
        #print('Backward')
        proto.setDemand(-512, TalonSrxProtocol.kThrottle)
    else:
        # Stop
        #print('Stop')
        proto.setDemand(0, TalonSrxProtocol.kThrottle)

    time.sleep(0.01)
    cycle_count += 1
    if cycle_count == 240:
        cycle_count = 0
