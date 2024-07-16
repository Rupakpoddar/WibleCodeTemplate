# Import necessary modules
from machine import Pin 
import bluetooth
from ble_simple_peripheral import BLESimplePeripheral
import time

# Create a Bluetooth Low Energy (BLE) object
ble = bluetooth.BLE()

# Create an instance of the BLESimplePeripheral class with the BLE object
sp = BLESimplePeripheral(ble)

# Set the print string time to 0.
print_string_time = 0

# Create a Pin object for the onboard LED, configure it as an output
led = Pin("LED", Pin.OUT)

# Initialize the LED state to 0 (off)
led_state = 0

# Define a callback function to handle received data
def on_rx(data):
    print("Data received: ", data)  # Print the received data
    global led_state  # Access the global variable led_state
    if data == b'toggle\r\n':  # Check if the received data is "toggle"
        led.value(not led_state)  # Toggle the LED state (on/off)
        led_state = 1 - led_state  # Update the LED state

# Start an infinite loop
while True:
    if sp.is_connected():  # Check if a BLE connection is established
        sp.on_write(on_rx)  # Set the callback function for data reception
    
    if (time.ticks_ms() - print_string_time > 1000):
        if sp.is_connected():
            # Create a message string
            msg="Hello_World!\n"
            # Send the message via BLE
            sp.send(msg)
            print("Data Sent")
        # Update the print string time    
        print_string_time = time.ticks_ms()