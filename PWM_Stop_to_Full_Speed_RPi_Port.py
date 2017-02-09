# Port of Stop_to_Full_Speed PWM program
from RPi import GPIO
from time import sleep

# Pin assignments
enA = 18
enB = 23
in1 = 5
in2 = 6
in3 = 13
in4 = 19

# PWM controllers (global)
pwmA = None
pwmB = None

# Setup I/O
def setup():
    global pwmA, pwmB
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(enA, GPIO.OUT)
    GPIO.setup(enB, GPIO.OUT)
    GPIO.setup(in1, GPIO.OUT)
    GPIO.setup(in2, GPIO.OUT)
    GPIO.setup(in3, GPIO.OUT)
    GPIO.setup(in4, GPIO.OUT)
    pwmA = GPIO.PWM(enA, 500)
    pwmB = GPIO.PWM(enB, 500)
    pwmA.start(0)
    pwmB.start(0)

# Loop process
def loop():
    GPIO.output(in1, GPIO.HIGH)
    GPIO.output(in2, GPIO.LOW)
    GPIO.output(in3, GPIO.HIGH)
    GPIO.output(in4, GPIO.LOW)

    for i in range(100):
        print('DC %d' % i)
        pwmA.ChangeDutyCycle(i)
        sleep(0.02)
        pwmB.ChangeDutyCycle(i)
        sleep(0.02)

    sleep(1.0)

# Enter program
setup()
while True:
    loop()
