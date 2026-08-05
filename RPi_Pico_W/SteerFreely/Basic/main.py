'''
    # Project Information:
        - Developer: Rupak Poddar
        - Wible Code Template for Raspberry Pi Pico W
        - Tested on: RP2040, RP2350
        - Steer Freely Basic & Motion Control Example

    # Instructions:
        - After flashing this code, open the Wible app on your smartphone.
        - Then select "Steer Freely" and look for the listing called "Wible".
'''

# Import necessary modules
import time
import bluetooth
from machine import Pin, PWM
from ble_simple_peripheral import BLESimplePeripheral

# Set verbose flag to enable print statements
VERBOSE = True

# Motor duty cycle limits and motion thresholds
MIN_DUTY = 20000
MAX_DUTY = 65535
PWM_FREQUENCY = 1000
MOTION_TIMEOUT_MS = 250
MOTION_X_UPPER_BOUND = 128 + 32
MOTION_X_LOWER_BOUND = 128 - 32
MOTION_Y_UPPER_BOUND = 128 + 32
MOTION_Y_LOWER_BOUND = 128 - 32

#                    FRONT
#                      ↑
#               ○ ┌─────────┐ ○
#              M1 │  ROBOT  │ M2
#               ○ └─────────┘ ○
#                      ↓
#                    BACK
#
#          A = Forward | B = Backward

# Initialize Pin objects for motor driver inputs and LEDs
LED_BUILTIN = Pin("LED", Pin.OUT)  # BLE connection status indicator
LIGHT = Pin(28, Pin.OUT)
M1A = PWM(Pin(2))
M1B = PWM(Pin(3))
M2A = PWM(Pin(4))
M2B = PWM(Pin(5))

M1A.freq(PWM_FREQUENCY)
M1B.freq(PWM_FREQUENCY)
M2A.freq(PWM_FREQUENCY)
M2B.freq(PWM_FREQUENCY)

# Create a Bluetooth Low Energy (BLE) object
ble = bluetooth.BLE()

# Create an instance of the BLESimplePeripheral class with the BLE object
sp = BLESimplePeripheral(ble)

# Global variables
motion_active = False
last_motion_command_time = 0


def map_range(x, in_min, in_max, out_min, out_max):
    return (x - in_min) * (out_max - out_min) // (in_max - in_min) + out_min


def to_int(value):
    try:
        return int(value)
    except ValueError:
        return 0


def stopMotors():
    M1A.duty_u16(0)
    M1B.duty_u16(0)
    M2A.duty_u16(0)
    M2B.duty_u16(0)


stopMotors()

print("\n\nWible RPi Pico W Steer Freely\n")


# Define a callback function to handle received data
def on_rx(data):
    global motion_active, last_motion_command_time

    try:
        rx_value = data.decode().strip()
    except Exception:
        rx_value = ""

    if len(rx_value) == 0:
        return

    if VERBOSE:
        print("\n----- Received string:", rx_value, "-----\n")

    duty = 0
    command = ""

    if len(rx_value) == 7:  # Button input
        # Parse rx_value in {SSS}{CCCC} format: SSS = speed, CCCC = command.
        duty = to_int(rx_value[0:3])
        command = rx_value[3:7]

        if duty != 0:
            duty = map_range(duty, 1, 100, MIN_DUTY, MAX_DUTY)

    elif len(rx_value) == 8:  # Motion input
        last_motion_command_time = time.ticks_ms()
        motion_active = True

        # Parse rx_value in X{AAA}Y{BBB} format: AAA = X-axis position from 0 to 255, BBB = Y-axis position from 0 to 255.
        x_value = to_int(rx_value[1:4])
        y_value = to_int(rx_value[5:8])

        # Phone position: Neutral
        if ((x_value <= MOTION_X_UPPER_BOUND)
                and (x_value >= MOTION_X_LOWER_BOUND)
                and (y_value <= MOTION_Y_UPPER_BOUND)
                and (y_value >= MOTION_Y_LOWER_BOUND)):
            stopMotors()
            return

        # Phone position: Forward
        if ((x_value > MOTION_X_UPPER_BOUND)
                and (y_value <= MOTION_Y_UPPER_BOUND)
                and (y_value >= MOTION_Y_LOWER_BOUND)):
            duty = map_range(x_value, MOTION_X_UPPER_BOUND, 255, MIN_DUTY, MAX_DUTY)
            command = "FWRD"

        # Phone position: Backward
        if ((x_value < MOTION_X_LOWER_BOUND)
                and (y_value <= MOTION_Y_UPPER_BOUND)
                and (y_value >= MOTION_Y_LOWER_BOUND)):
            duty = map_range(MOTION_X_LOWER_BOUND - x_value, 0, MOTION_X_LOWER_BOUND, MIN_DUTY, MAX_DUTY)
            command = "BKWD"

        # Phone position: Left
        if ((x_value <= MOTION_X_UPPER_BOUND)
                and (x_value >= MOTION_X_LOWER_BOUND)
                and (y_value < MOTION_Y_LOWER_BOUND)):
            duty = map_range(MOTION_Y_LOWER_BOUND - y_value, 0, MOTION_Y_LOWER_BOUND, MIN_DUTY, MAX_DUTY)
            command = "LEFT"

        # Phone position: Right
        if ((x_value <= MOTION_X_UPPER_BOUND)
                and (x_value >= MOTION_X_LOWER_BOUND)
                and (y_value > MOTION_Y_UPPER_BOUND)):
            duty = map_range(y_value, MOTION_Y_UPPER_BOUND, 255, MIN_DUTY, MAX_DUTY)
            command = "RGHT"

        # Phone position: Front left
        if ((x_value > MOTION_X_UPPER_BOUND)
                and (y_value < MOTION_Y_LOWER_BOUND)):
            duty = map_range(x_value, MOTION_X_UPPER_BOUND, 255, MIN_DUTY, MAX_DUTY)
            command = "FRLT"

        # Phone position: Front right
        if ((x_value > MOTION_X_UPPER_BOUND)
                and (y_value > MOTION_Y_UPPER_BOUND)):
            duty = map_range(x_value, MOTION_X_UPPER_BOUND, 255, MIN_DUTY, MAX_DUTY)
            command = "FRRT"

        # Phone position: Back left
        if ((x_value < MOTION_X_LOWER_BOUND)
                and (y_value < MOTION_Y_LOWER_BOUND)):
            duty = map_range(MOTION_X_LOWER_BOUND - x_value, 0, MOTION_X_LOWER_BOUND, MIN_DUTY, MAX_DUTY)
            command = "BKLT"

        # Phone position: Back right
        if ((x_value < MOTION_X_LOWER_BOUND)
                and (y_value > MOTION_Y_UPPER_BOUND)):
            duty = map_range(MOTION_X_LOWER_BOUND - x_value, 0, MOTION_X_LOWER_BOUND, MIN_DUTY, MAX_DUTY)
            command = "BKRT"

    else:
        stopMotors()
        if VERBOSE:
            print("Invalid input")
        return

    if VERBOSE:
        print("Duty:\t\t", duty)
        print("Command:\t", command, "\n")

    # STOP
    if command == "STOP":
        stopMotors()

    # LED ON
    if command == "LTON":
        LIGHT.on()

    # LED OFF
    if command == "LTOF":
        LIGHT.off()

    # FORWARD
    if command == "FWRD":
        M1A.duty_u16(duty)
        M2A.duty_u16(duty)
        M1B.duty_u16(0)
        M2B.duty_u16(0)

    # BACKWARD
    if command == "BKWD":
        M1A.duty_u16(0)
        M2A.duty_u16(0)
        M1B.duty_u16(duty)
        M2B.duty_u16(duty)

    # LEFT
    if command == "LEFT":
        M1A.duty_u16(0)
        M2A.duty_u16(duty)
        M1B.duty_u16(duty)
        M2B.duty_u16(0)

    # RIGHT
    if command == "RGHT":
        M1A.duty_u16(duty)
        M2A.duty_u16(0)
        M1B.duty_u16(0)
        M2B.duty_u16(duty)

    ### Utilize additional control buttons and multi-touch commands from below ###

    # ROTATE LEFT
    # if command == "ROLT":
    #     pass

    # ROTATE RIGHT
    # if command == "RORT":
    #     pass

    # FORWARD + LEFT
    if command == "FRLT":
        M1A.duty_u16(duty // 2)
        M2A.duty_u16(duty)
        M1B.duty_u16(0)
        M2B.duty_u16(0)

    # FORWARD + RIGHT
    if command == "FRRT":
        M1A.duty_u16(duty)
        M2A.duty_u16(duty // 2)
        M1B.duty_u16(0)
        M2B.duty_u16(0)

    # FORWARD + ROTATE LEFT
    # if command == "FRRL":
    #     pass

    # FORWARD + ROTATE RIGHT
    # if command == "FRRR":
    #     pass

    # BACKWARD + LEFT
    if command == "BKLT":
        M1A.duty_u16(0)
        M2A.duty_u16(0)
        M1B.duty_u16(duty // 2)
        M2B.duty_u16(duty)

    # BACKWARD + RIGHT
    if command == "BKRT":
        M1A.duty_u16(0)
        M2A.duty_u16(0)
        M1B.duty_u16(duty)
        M2B.duty_u16(duty // 2)

    # BACKWARD + ROTATE LEFT
    # if command == "BKRL":
    #     pass

    # BACKWARD + ROTATE RIGHT
    # if command == "BKRR":
    #     pass

    ### Utilize additional function buttons from below ###

    # FUNCTION 1
    # if command == "FNC1":
    #     pass

    # FUNCTION 2
    # if command == "FNC2":
    #     pass

    # FUNCTION 3
    # if command == "FNC3":
    #     pass

    # FUNCTION 4
    # if command == "FNC4":
    #     pass


# Set the callback function for data reception
sp.on_write(on_rx)

# Start an infinite loop
while True:
    if not sp.is_connected():  # Check if BLE connection is established
        stopMotors()

        LED_BUILTIN.off()
        time.sleep(0.25)
        LED_BUILTIN.on()
        time.sleep(0.25)

    if motion_active:
        if time.ticks_diff(time.ticks_ms(), last_motion_command_time) > MOTION_TIMEOUT_MS:
            motion_active = False
            stopMotors()

    time.sleep_ms(10)
