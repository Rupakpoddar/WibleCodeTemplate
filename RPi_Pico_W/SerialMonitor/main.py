'''
    # Project Information:
        - Developer: Rupak Poddar
        - Wible Code Template for Raspberry Pi Pico W
        - Tested on: RP2040, RP2350
        - Serial Monitor Example

    # Instructions:
        - After flashing this code, open the Wible app on your smartphone.
        - Then select "Serial Monitor" and look for the listing called "Wible".
'''

# Import necessary modules
import time
import bluetooth
from machine import Pin 
from ble_simple_peripheral import BLESimplePeripheral

# Create a Bluetooth Low Energy (BLE) object
ble = bluetooth.BLE()

# Create an instance of the BLESimplePeripheral class with the BLE object
sp = BLESimplePeripheral(ble)

# Initialize counter
counter = 0

# Implement delay without blocking
previousMillis = 0  # Stores the last time the event occurred
interval = 1000 # Interval for the delay in milliseconds

# Create a Pin object for the onboard LED, configure it as an output
LED_BUILTIN = machine.Pin("LED", machine.Pin.OUT) # BLE connection status indicator

print("\n\nWible RPi Pico W Serial Monitor\n")

# Define a callback function to handle received data
def on_rx(data):
    data = str(data)[2:-1]  # Remove unwanted characters
    print("Data Received: ", data)  # Print the received data

# Start an infinite loop
while True:
    if sp.is_connected():  # Check if BLE connection is established
        sp.on_write(on_rx)  # Set the callback function for data reception

        currentMillis = time.ticks_ms()

        if (currentMillis - previousMillis > interval): # Check if the interval has passed
            # Save the last time the event occurred
            previousMillis = currentMillis

            # Create a message string
            msg = "Counter value: " + str(counter) + "\n"

            # Send the message via BLE
            sp.send(msg)

            # Increment the counter
            counter += 1
    
    else:
        # Blink the builtin LED while the BLE device is waiting for connection
        LED_BUILTIN.off()
        time.sleep(0.25)
        LED_BUILTIN.on()
        time.sleep(0.25)
