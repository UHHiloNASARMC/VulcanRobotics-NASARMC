from RPi import GPIO
BRAKE_PIN = 24
GPIO.setmode(GPIO.BCM)
GPIO.setup(BRAKE_PIN, GPIO.OUT)
import talonsrx, time

intf = talonsrx.TalonCANInterface()
proto = talonsrx.TalonSrxProtocol(intf, 4)
cycle_count = 0
brake_state = True
for val in range(0, 1024):
    print('THROTTLE %d' % val)
    proto.setDemand(val, talonsrx.TalonSrxProtocol.kThrottle)
    time.sleep(0.01)

    if not cycle_count % 50:
        brake_state ^= True
        if brake_state:
            GPIO.output(BRAKE_PIN, GPIO.LOW)
        else:
            GPIO.output(BRAKE_PIN, GPIO.HIGH)
        print('BRAKE', brake_state)
    cycle_count += 1
