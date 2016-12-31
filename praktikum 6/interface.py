import serial
import time
ser = serial.Serial('COM4', 9600)  # open serial port
varlala = "___a\nkwkwkw"
ser.write(str.encode(varlala))
time.sleep(1)
varb = "huehuhe"
while ser.inWaiting() > 0:
	varb = ser.read(1)
	print(varb)
#ser.write(b'test')     # write a string
ser.close()             # close port