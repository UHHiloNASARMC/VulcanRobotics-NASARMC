# Released by rdb under the Unlicense (unlicense.org)
# Based on information from:
# https://www.kernel.org/doc/Documentation/input/joystick-api.txt

import os, struct, array, time
from fcntl import ioctl
import talonsrx

# Iterate over the joystick devices.
print('Available devices:')

for fn in os.listdir('/dev/input'):
    if fn.startswith('js'):
        print('  /dev/input/%s' % (fn))

# We'll store the states here.
axis_states = {}
button_states = {}

# These constants were borrowed from linux/input.h
axis_names = {
    0x00 : 'x',
    0x01 : 'y',
    0x02 : 'z',
    0x03 : 'rx',
    0x04 : 'ry',
    0x05 : 'rz',
    0x06 : 'trottle',
    0x07 : 'rudder',
    0x08 : 'wheel',
    0x09 : 'gas',
    0x0a : 'brake',
    0x10 : 'hat0x',
    0x11 : 'hat0y',
    0x12 : 'hat1x',
    0x13 : 'hat1y',
    0x14 : 'hat2x',
    0x15 : 'hat2y',
    0x16 : 'hat3x',
    0x17 : 'hat3y',
    0x18 : 'pressure',
    0x19 : 'distance',
    0x1a : 'tilt_x',
    0x1b : 'tilt_y',
    0x1c : 'tool_width',
    0x20 : 'volume',
    0x28 : 'misc',
}

button_names = {
    0x120 : 'trigger',
    0x121 : 'thumb',
    0x122 : 'thumb2',
    0x123 : 'top',
    0x124 : 'top2',
    0x125 : 'pinkie',
    0x126 : 'base',
    0x127 : 'base2',
    0x128 : 'base3',
    0x129 : 'base4',
    0x12a : 'base5',
    0x12b : 'base6',
    0x12f : 'dead',
    0x130 : 'a',
    0x131 : 'b',
    0x132 : 'c',
    0x133 : 'x',
    0x134 : 'y',
    0x135 : 'z',
    0x136 : 'tl',
    0x137 : 'tr',
    0x138 : 'tl2',
    0x139 : 'tr2',
    0x13a : 'select',
    0x13b : 'start',
    0x13c : 'mode',
    0x13d : 'thumbl',
    0x13e : 'thumbr',

    0x220 : 'dpad_up',
    0x221 : 'dpad_down',
    0x222 : 'dpad_left',
    0x223 : 'dpad_right',

    # XBox 360 controller uses these codes.
    0x2c0 : 'dpad_left',
    0x2c1 : 'dpad_right',
    0x2c2 : 'dpad_up',
    0x2c3 : 'dpad_down',
}

axis_map = []
button_map = []

# Open the joystick device.
fn = '/dev/input/js0'
print('Opening %s...' % fn)
jsdev = open(fn, 'rb')

# Get the device name.
#buf = bytearray(63)
buf = array.array('B', [0] * 64)
ioctl(jsdev, 0x80006a13 + (0x10000 * len(buf)), buf) # JSIOCGNAME(len)
js_name = buf.tostring()
print('Device name: %s' % js_name)

# Get number of axes and buttons.
buf = array.array('B', [0])
ioctl(jsdev, 0x80016a11, buf) # JSIOCGAXES
num_axes = buf[0]

buf = array.array('B', [0])
ioctl(jsdev, 0x80016a12, buf) # JSIOCGBUTTONS
num_buttons = buf[0]

# Get the axis map.
buf = array.array('B', [0] * 0x40)
ioctl(jsdev, 0x80406a32, buf) # JSIOCGAXMAP

for axis in buf[:num_axes]:
    axis_name = axis_names.get(axis, 'unknown(0x%02x)' % axis)
    axis_map.append(axis_name)
    axis_states[axis_name] = 0.0

# Get the button map.
buf = array.array('H', [0] * 200)
ioctl(jsdev, 0x80406a34, buf) # JSIOCGBTNMAP

for btn in buf[:num_buttons]:
    btn_name = button_names.get(btn, 'unknown(0x%03x)' % btn)
    button_map.append(btn_name)
    button_states[btn_name] = 0

print('%d axes found: %s' % (num_axes, ', '.join(axis_map)))
print('%d buttons found: %s' % (num_buttons, ', '.join(button_map)))

# Setup talon interface
intf = talonsrx.TalonCANInterface()
front_left = talonsrx.TalonSrxProtocol(intf, 1)
front_right = talonsrx.TalonSrxProtocol(intf, 2)
back_left = talonsrx.TalonSrxProtocol(intf, 3)
back_right = talonsrx.TalonSrxProtocol(intf, 4)

# Disable drive functionality
disabled = True
enable_start_time = None
def set_disable(disable):
    global disabled
    disabled = disable
    if disable:
        print('DISABLED')
        front_left.setControlMode(talonsrx.TalonSrxProtocol.kDisabled)
        front_right.setControlMode(talonsrx.TalonSrxProtocol.kDisabled)
        back_left.setControlMode(talonsrx.TalonSrxProtocol.kDisabled)
        back_right.setControlMode(talonsrx.TalonSrxProtocol.kDisabled)
    else:
        print('ENABLED')

# Main event loop
while True:
    evbuf = jsdev.read(8)
    if evbuf:
        evtime, value, type, number = struct.unpack('IhBB', evbuf)

        #if type & 0x80:
        #     print("(initial)")

        if type & 0x01:
            button = button_map[number]
            if button:
                button_states[button] = value
                #if value:
                #    print("%s pressed" % (button))
                #else:
                #    print("%s released" % (button))

                # Circle button
                if number == 13 and value:
                    set_disable(True)

                # PS button
                if number == 16:
                    if value:
                        # Mark hold start time
                        print('HOLD FOR 2 SEC')
                        set_disable(True)
                        enable_start_time = time.time()
                    elif enable_start_time:
                        if (time.time() - enable_start_time) > 2.0:
                            # Button released after 2 seconds; enable control
                            set_disable(False)
                        else:
                            # Button released before or at 2 seconds; cancel enable
                            print('CANCELLED')
                            enable_start_time = None
                    

        if type & 0x02:
            axis = axis_map[number]
            if axis:
                fvalue = value / 32767.0
                axis_states[axis] = fvalue
                #if number in (0,1,2,3):
                #    print("%s: %.3f" % (axis, fvalue))

            if not disabled:
                if number == 1: # Left Stick Y
                    fvalue = min(1023, max(-1023, -value // 128))
                    print('LEFT', fvalue)
                    front_left.setDemand(fvalue, talonsrx.TalonSrxProtocol.kThrottle)
                    back_left.setDemand(fvalue, talonsrx.TalonSrxProtocol.kThrottle)
                    
                if number == 3: # Right Stick Y
                    fvalue = min(1023, max(-1023, -value // 128))
                    print('RIGHT', fvalue)
                    front_right.setDemand(fvalue, talonsrx.TalonSrxProtocol.kThrottle)
                    back_right.setDemand(fvalue, talonsrx.TalonSrxProtocol.kThrottle)

