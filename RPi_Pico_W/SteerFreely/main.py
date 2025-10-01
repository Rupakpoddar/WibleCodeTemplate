'''
    # Project Information:
        - Developer: Rupak Poddar
        - Wible Code Template for Raspberry Pi Pico W
        - Tested on: RP2040, RP2350
        - Steer Freely Example

    # Instructions:
        - After flashing this code, open the Wible app on your smartphone.
        - Then select "Steer Freely" and look for the listing called "Wible".
'''

# Import necessary modules
import time
import bluetooth
from machine import Pin, PWM 
from ble_simple_peripheral import BLESimplePeripheral

# Create a Bluetooth Low Energy (BLE) object
ble = bluetooth.BLE()

# Create an instance of the BLESimplePeripheral class with the BLE object
sp = BLESimplePeripheral(ble)

# Initial ignition state
ignitionState = False

# Initialize Pin objects for motor driver inputs and LEDs
LED_BUILTIN = machine.Pin("LED", machine.Pin.OUT) # BLE connection status indicator
LIGHT = Pin(28, Pin.OUT)
M1A = PWM(Pin(2))
M1B = PWM(Pin(3))
M2A = PWM(Pin(4))
M2B = PWM(Pin(5))

M1A.freq(1000)
M1B.freq(1000)
M2A.freq(1000)
M2B.freq(1000)

def map_range(x, in_min, in_max, out_min, out_max):
  return (x - in_min) * (out_max - out_min) // (in_max - in_min) + out_min

def _stop():
    M1A.duty_u16(0)
    M1B.duty_u16(0)
    M2A.duty_u16(0)
    M2B.duty_u16(0)

print("\n\nWible RPi Pico W Steer Freely\n")

# Define a callback function to handle received data
def on_rx(data):
    global ignitionState
    speed = int(str(data)[2:5])
    command = str(data)[5:-1]
    duty = map_range(speed, 0, 255, 0, 65025)

    print("----- Data received: ", data, " -----\n")
    print("Speed:\t\t", speed)
    print("Command:\t", command)
    print("Duty:\t\t", duty, "\n")
    
    if command == "IGON":
        ignitionState = True
    
    if command == "IGOF":
        ignitionState = False
        _stop()
    
    if command == "STOP":
        _stop()
    
    if command == "LTON":
        LIGHT.on()
    
    if command == "LTOF":
        LIGHT.off()
    
    '''    
    ### Utilize the auxiliary buttons from below ###
    
    if command == "AUX1":
        pass
    
    if command == "AUX2":
        pass
    
    if command == "AUX3":
        pass
    
    if command == "AUX4":
        pass
    
    if command == "AUX5":
        pass
    
    if command == "AUX6":
        pass
    '''
    
    ### Uncomment the following line to disable ignition button ###
    # ignitionState = True
    
    if ignitionState:
        #
        # FORWARD
        #
        if command == "FWRD":
            M1A.duty_u16(duty)
            M1B.duty_u16(0)
            M2A.duty_u16(duty)
            M2B.duty_u16(0)
        
        #
        # BACKWARD
        #
        if command == "BKWD":
            M1A.duty_u16(0)
            M1B.duty_u16(duty)
            M2A.duty_u16(0)
            M2B.duty_u16(duty)
        
        #
        # LEFT
        #
        if command == "LEFT":
            M1A.duty_u16(0)
            M1B.duty_u16(duty)
            M2A.duty_u16(duty)
            M2B.duty_u16(0)
        
        #
        # RIGHT
        #
        if command == "RGHT":
            M1A.duty_u16(duty)
            M1B.duty_u16(0)
            M2A.duty_u16(0)
            M2B.duty_u16(duty)
        
        '''    
        ### Utilize the multi-touch capability of the app from below ###
        
        if command == "FRLT": # FORWARD + LEFT
            pass
        
        if command == "FRRT": # FORWARD + RIGHT
            pass
        
        if command == "BKLT": # BACKWARD + LEFT
            pass
        
        if command == "BKRT": # BACKWARD + RIGHT
            pass
        '''
    else:
        print("+------------------------+")
        print("|                        |")
        print("|   IGNITION STATE OFF   |")
        print("|                        |")
        print("+------------------------+\n")

# Start an infinite loop
while True:
    if sp.is_connected():  # Check if BLE connection is established
        sp.on_write(on_rx)  # Set the callback function for data reception
    
    else:
        # Waiting for connection
        ignitionState = False
        _stop()

        LED_BUILTIN.off()
        time.sleep(0.25)
        LED_BUILTIN.on()
        time.sleep(0.25)
