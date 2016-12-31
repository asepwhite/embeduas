import serial
ser = serial.Serial('COM5', 115200)  # open serial port
print(ser.name)         # check which port was really used
ser.write(b'test')     # write a string
ser.close()             # close port