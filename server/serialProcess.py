import serial
import time
import multiprocessing
import mod_pubvars as m
import debug_helper as dh
class SerialProcess(multiprocessing.Process):

    def __init__(self, taskQ, resultQ):
        multiprocessing.Process.__init__(self)
        self.taskQ = taskQ
        self.resultQ = resultQ
        #https://www.raspberrypi.org/forums/viewtopic.php?f=66&t=64493
        # in command do: ls /dev/serial/by-id/   when arduino is connected
        self.usbPort = '/dev/serial/by-id/usb-Arduino__www.arduino.cc__0043_64131383231351D0F042-if00' #' '/dev/npboard' #'/dev/ttyUSB0' # '/dev/ttyACM0'
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
                dh.dbg("Serial Worker Xmit : " + task )

            # look for incoming serial data
            try:
                if (self.sp.inWaiting() > 0):
                    result = self.sp.readline()
                    #print "Serial Worker Received : " + result
                    result = result.replace("\n", "")
                   # print "Serial Worker Received2 : " + result
                    # send it back to tornado
                    self.resultQ.put(result)
            except:
                print "error -> m.ser_stat=2"
                #self.sp.close()
                m.ser_stat=2
                break