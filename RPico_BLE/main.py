# Import necessary modules
import time
import bluetooth
from machine import Pin 
from ble_simple_peripheral import BLESimplePeripheral

# Create a Bluetooth Low Energy (BLE) object
ble = bluetooth.BLE()

# Create an instance of the BLESimplePeripheral class with the BLE object
sp = BLESimplePeripheral(ble)

# Initialize Pin objects for motor driver inputs and LEDs
LED_BUILTIN = machine.Pin("LED", machine.Pin.OUT) # BLE connection status indicator

# Define a callback function to handle received data
def on_rx(data):
    print("Data received: ", data)  # Print the received data
    
    if data == b'100LTON':
        LED_BUILTIN.on()
    
    if data == b'100LTOF':  # Check if the received data is "toggle"
        LED_BUILTIN.off()

# Start an infinite loop
while True:
    if sp.is_connected():  # Check if a BLE connection is established
        sp.on_write(on_rx)  # Set the callback function for data reception
    else:
        LED_BUILTIN.off()
        time.sleep(0.1)
        LED_BUILTIN.on()
        time.sleep(0.1)