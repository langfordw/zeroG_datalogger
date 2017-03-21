import sys
import serial
import numpy as np
from matplotlib import pyplot as plt

f = open('datalog.dat','w')

if (len(sys.argv) != 3):
   print "command line: zerog_datalogger.py serial_port baud"
   sys.exit()

port = sys.argv[1]
baud = sys.argv[2]

ser = serial.Serial(port, baud)

while True:  
    try:
      ser.flush()
      print("here")
        
    except KeyboardInterrupt, e:
        # for i in range(len(xx)):
        #     f.write('%5.5f \t %d \n' %(xx[i], yy[i]))
        print 'saving data and exiting'
        break
    