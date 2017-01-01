# import depececies module
import serial
import time
# create usb connection
ser = serial.Serial('COM5', 9600)  # open serial port
varinfinite = 1 #constant for infinite loop
# Open a file
fo = open("log.txt", "a+")
# infinite loop command line interface
# consist of 3 type command 
# input 0 considered as exiting interface, input 1-5 as valid command and input > 6 sa invalid command
while varinfinite == 1:
    command = input("***** Command Line Interface *****\nFeature : \n1.Free Heap\n2.Qtouch\n3.Light Sensor\n4.Potentio\n5.Servo\n6.Helmet Status\nPlease Input Your Command :\n")
    command = int(command)
    fo.write("***** Command Line Interface *****\nFeature : \n1.Free Heap\n2.Qtouch\n3.Light Sensor\n4.Potentio\n5.Servo\n6.Helmet Status\nPlease Input Your Command :\n")
    if command > 6:
        print("Invalid Command, Please Try Again.....")
        fo.write("Invalid Command, Please Try Again.....")
    elif command == 0:
        print("Exiting Command Line Interface.....")
        fo.write("Exiting Command Line Interface.....")
        varinfinite = 0
    else:
        command = str(command)        
        command = "___"+command
        varsendtime = time.time();
        ser.write(str.encode(command))
        time.sleep(0.5)
        varresult = ""
        while ser.inWaiting() > 0:
            vartemp = ser.read(1)
            vartemp = vartemp.decode("utf-8")
            varresult = varresult+vartemp
        varreceivetime = time.time();
        vartimediff = str((varreceivetime - varsendtime) * 1000)
        print(varresult)
        fo.write(varresult+"\n")    
        print("Round Trip Time : "+vartimediff)
        fo.write("Round Trip Time : "+vartimediff)
        ser.flush()
        ser.reset_input_buffer()
        ser.reset_output_buffer()
        #ser.write(b'test')     # write a string
# Close opend file
fo.close()
ser.close()             # close port