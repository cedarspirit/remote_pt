#for client - arduino connect and has a pot
import serial
import time
import multiprocessing
 
class SerialProcess(multiprocessing.Process):
 
    def __init__(self, serPort, taskQ, resultQ):
        multiprocessing.Process.__init__(self)
        self.taskQ = taskQ
        self.resultQ = resultQ
        self.usbPort = serPort # '/dev/ttyUSB0'
        self.sp = serial.Serial(self.usbPort, 9600, timeout=1)
 
    def close(self):
        self.sp.close()
 
    def sendData(self, data):
        print "sendData start..."
        self.sp.write(data)
        time.sleep(3)
        print "sendData done: " + data
 
    def run(self):
 
    	self.sp.flushInput()
 
        while True:
            # look for incoming tornado request
            if not self.taskQ.empty():
                task = self.taskQ.get()
 
                # send it to the arduino
                self.sp.write(task + "\n");
                print "sending to local arduino: " + task
 
            # look for incoming serial data
            if (self.sp.inWaiting() > 0):
            	#result = self.sp.readline().replace("\n", "")
                result = self.sp.readline()
                # send it back to tornado
            	self.resultQ.put(result)