#!/usr/bin/env python

import sys
import logging
import time
import thread
import socket
import json
import tornado.websocket
import tornado.ioloop
import tornado.gen
import tornado.autoreload
import serialHandler
import multiprocessing

from tornado.options import define, options
from config import PORT_COUNT, WS_PORT, HOST_IP

# LOG_FILENAME = 'example.log'
#logging.basicConfig(filename=LOG_FILENAME,level=logging.DEBUG)
#logging.debug('This message should go to the log file')

# For displaying throughput
processed_requests, processed_bytes = 0, 0
last_display_time = time.time()

define("wsport", default=WS_PORT, help="WebSocket port", type=int)
define("host", default=HOST_IP, help="Server IP", type=str)
options.parse_command_line(sys.argv)

connections = set()
conn_try_count = 0
write_try_count = 0

rxBuffer = ''


LastTick = 0
curMode = 0

@tornado.gen.coroutine
def loop_websocket(ws):
    global write_try_count
    global curMode
    global LastTick
    while True:
        if ws.stream.closed():
            logging.info("loop out - closed!")
            try:
                connections.remove(ws)
            except:
                pass
            break
        data = yield ws.read_message()
        write_try_count -= 1
        if data:
            LastTick = 0
            if curMode != 1:
                curMode = 1
            logging.info("on_message (%d): " % (len(data),))
            print ("on_message (%d): " % (len(data),))
            print (data)


@tornado.gen.coroutine
def make_websocket_connection(host, port):
    global LastTick
    global curMode

    LastTick = 0
    curMode = 0

    url = "ws://%s:%d/ws" % (host, port)
    print "++++ CONNECTING TO +++" + url
    try:
        ws = yield tornado.websocket.websocket_connect(url)
        ws.stream.socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        connections.add(ws)
        loop_websocket(ws)
        curMode = 1
    except:
        curMode = 2
        print("errrrr")

def close_all_connections():
    while connections:
        ws = connections.pop()
        ws.close()
        time.sleep(0)



def connect_me():
    host = options.host
    port = options.wsport
    try:
        time.sleep(0.001)
        tornado.ioloop.IOLoop.current().add_callback(make_websocket_connection, *(host, port ))
    except:
        logging.warn("Error occured in connection.")
        import traceback
        traceback.print_exc()


def console_io_loop():
    global conn_try_count
    while True:
        logging.warn("Concurrent conns: %d" % (len(connections), ))
        logging.warn("(C)reate WS connections")
        logging.warn("(S)end messages to opened connections")
        logging.warn("(R)emove All connections")
        logging.warn("(Q)uit")
        line = sys.stdin.readline().strip().lower()
        starting = time.time()
        if not line:
            continue
        elif line[0] == "s":
            ###logging.warn("Input message length: ")
            ###message_length = int(sys.stdin.readline().strip())
            message = json.dumps({'id': 'begin', 'b': (2, 4), 'c': 3.0})
            starting = time.time()
            for i, ws in enumerate(connections):
                while write_try_count > 100:
                    time.sleep(0.001)
                tornado.ioloop.IOLoop.current().add_callback(
                    ws.write_message,
                    *(message,)
                )
            logging.warn("Sent to %d connections (%d bytes)" % (len(connections), len(connections) * len(message)))
        elif line[0] == "t":
            ###logging.warn("Input message length: ")
            ###message_length = int(sys.stdin.readline().strip())
            message = json.dumps({'id': 'sensor_t'})
            starting = time.time()
            for i, ws in enumerate(connections):
                while write_try_count > 100:
                    time.sleep(0.001)
                tornado.ioloop.IOLoop.current().add_callback(
                    ws.write_message,
                    *(message,)
                )
            logging.warn("Sent to %d connections (%d bytes)" % (len(connections), len(connections) * len(message)))
        elif line[0] == "r":
            close_all_connections()
        elif line[0] == "q":
            tornado.ioloop.IOLoop.current().add_callback(
                close_all_connections
            )
            logging.warn("elapsed: %f" % (time.time() - starting))
            tornado.ioloop.IOLoop.instance().stop()
            break
        logging.warn("elapsed: %f" % (time.time() - starting))


def reload_main():
    logging.warn("")
    logging.warn("Reload...")
    logging.warn("")

def main():
    taskQ = multiprocessing.Queue()
    comRcvQ = multiprocessing.Queue()

    sp = serialHandler.SerialProcess('/dev/ttyUSB0',taskQ, comRcvQ)
    sp.daemon = True
    sp.start()

    def HeartBeat():
        global LastTick
        global curMode
        LastTick += 1
        if curMode == 2:
            if LastTick > 3:
                LastTick=0
                connect_me()
        elif curMode == 1:
            if LastTick > 3: #it has been a while communicating => Send HELLO
                curMode = 3
                LastTick = 0
                sendHello()
        elif curMode == 3:
            if LastTick > 3:
                closeConnection()
                curMode = 2



        print "Heart Beat " + str(curMode)

    def closeConnection():
        close_all_connections()

    def sendHello():
            message = json.dumps({'id': 'C0'})
            starting = time.time()
            for i, ws in enumerate(connections):
                while write_try_count > 100:
                    time.sleep(0.001)
                tornado.ioloop.IOLoop.current().add_callback(
                    ws.write_message,
                    *(message,)
                )

    def checkComRcvQ():
        global rxBuffer
        if not comRcvQ.empty():
            result = rxBuffer + comRcvQ.get()
#             print "tornado received from arduino: " + result

            rx = result.split('\r\n' )
            rxBuffer = rx[-1]
            rx.pop() # remove last element
            for n in range(0, len(rx)):
                if rx[n]=='<X0>':
                    sp.taskQ.put("<Y" + str(curMode) +">\n")
                elif rx[n][0:4]=='<X1_':
                    pt=rx[n].split('_')
                    message = json.dumps({'id': 'C1', 'x': pt[1], 'y': pt[2]})
                    starting = time.time()
                    for i, ws in enumerate(connections):
                        while write_try_count > 100:
                            time.sleep(0.001)
                        tornado.ioloop.IOLoop.current().add_callback\
                            (
                                ws.write_message,
                                *(message,)
                            )

                ###print "ZZZ " + rx[n]

            # logging.warn("Sent to %d connections (%d bytes)" % (len(connections), len(connections) * len(message)))





    logging.getLogger().setLevel(logging.WARN)
    logging.warn("WS Client: Test C1M on Tornado")
    logging.warn("------------------------------")
    logging.warn("")




   ### tornado.autoreload.add_reload_hook(reload_main)
   ### tornado.autoreload.start()

    thread.start_new_thread(console_io_loop, ())
    connect_me()

    mainLoop = tornado.ioloop.IOLoop.instance()
    scheduler = tornado.ioloop.PeriodicCallback(checkComRcvQ, 10, io_loop = mainLoop)
    scheduler.start()


    HeartBeatLoop = tornado.ioloop.IOLoop.instance()
    scheduler2 = tornado.ioloop.PeriodicCallback(HeartBeat, 2000, io_loop = HeartBeatLoop)
    scheduler2.start()

    tornado.ioloop.IOLoop.instance().start()

if __name__ == "__main__":
    main()
