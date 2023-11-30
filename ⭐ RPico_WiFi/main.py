import network
import socket
from time import sleep
from machine import Pin, PWM

#
# Enter your WiFi credentials 
#
ssid = 'ENTER YOUR SSID'
password = 'ENTER YOUR PASSWORD'

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
M1A.freq(1000)
M1A.freq(1000)

def map_range(x, in_min, in_max, out_min, out_max):
  return (x - in_min) * (out_max - out_min) // (in_max - in_min) + out_min

def _stop():
    M1A.duty_u16(0)
    M1B.duty_u16(0)
    M2A.duty_u16(0)
    M2B.duty_u16(0)

_stop()

wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect(ssid, password)

max_wait = 10
while max_wait > 0:
    if wlan.status() < 0 or wlan.status() >= 3:
        break
    max_wait -= 1
    print('waiting for connection...')
    LED_BUILTIN.off()
    sleep(0.5)
    LED_BUILTIN.on()
    sleep(0.5)

if wlan.status() != 3:
    raise RuntimeError('network connection failed')
else:
    print('connected')
    status = wlan.ifconfig()
    print( 'ip = ' + status[0] )

addr = socket.getaddrinfo('0.0.0.0', 80)[0][-1]

s = socket.socket()
s.bind(addr)
s.listen(1)

print('listening on', addr, '\n')

# Listen for connections
while True:
    try:
        cl, addr = s.accept()
        print('client connected from', addr)
        request = cl.recv(1024)
        # print(request)

        request = str(request)
        request = request[request.find('/')+1:request.find('/')+8]
        speed = int(request[0:3])
        command = str(request)[3:]
        duty = map_range(speed, 0, 255, 0, 65025)

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
        ### Utilize the Auxiliary buttons from below ###
        
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
                M1A.duty_u16(duty)
                M1B.duty_u16(0)
                M2A.duty_u16(0)
                M2B.duty_u16(duty)
            
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

        response = request

        cl.send('HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n')
        cl.send(response)
        cl.close()

    except OSError as e:
        _stop()
        cl.close()
        print('connection closed')