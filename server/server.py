#!/usr/bin/python27
import json
import tornado.httpserver
import tornado.ioloop
import tornado.web
import tornado.websocket
import tornado.gen
from tornado.options import define, options
 
import time
import multiprocessing
import serialProcess
 
define("port", default=8080, help="run on the given port", type=int)
 
clients = []
taskQ = multiprocessing.Queue()
resultQ = multiprocessing.Queue()


class IndexHandler(tornado.web.RequestHandler):
    def get(self):
        self.render('index.html')
 
class WSHandler(tornado.websocket.WebSocketHandler):
    def check_origin(self, origin):
        return True

    def open(self):
        print 'new connection'
        clients.append(self)
        self.write_message("connected")

    def on_message(self, message):
        print 'tornado received from UI controller: %s' % message

        try:
            cm = json.loads(message)

            if cm['id']=='C0': #Hello
                print 'rcvd Hello Message'
                self.write_message(json.dumps({'id': 'S0'}))
            elif cm['id']=='C1': #PT Data
                print 'rcvd PT Message : x=' + cm['x'] + ' y=' + cm['y']
                #self.write_message('got it! ' + cm['id'])
                q = self.application.settings.get('queue')
                q.put("<A1_" + cm['x']  +'_' + cm['y'] + "_>\n")
                send2all (json.dumps({'id': 'Z3','x':cm['x'],'y':cm['y']}))

            if cm['id']=='D1': #Hello
                send2all (json.dumps({'id': 'Z3','x':cm['x'],'y':cm['y']}))
                print '======================================'
                #self.write_message(json.dumps({'id': 'S0'}))
        except:
            print 'invalid JSON message'



    def on_close(self):
        print 'connection closed'
        clients.remove(self)

class WSH(tornado.websocket.WebSocketHandler):
    def check_origin(self, origin):
        return True

    def open(self):
        print 'new connection'
        clients.append(self)
        self.write_message("connected")

    def on_message(self, message):
        print 'tornado received from web APP: %s' % message

        try:
            cm = json.loads(message)

            if cm['id']=='C0': #Hello
                print 'rcvd Hello Message'
                self.write_message(json.dumps({'id': 'S0'}))
            elif cm['id']=='C1': #PT Data
                print 'rcvd PT Message : x=' + cm['x'] + ' y=' + cm['y']
                self.write_message('got it! ' + cm['id'])
                q = self.application.settings.get('queue')
               # q.put(message)
                q.put("<A1_" + cm['x']  +'_' + cm['y'] + "_>\n")
            if cm['id']=='D1': #Hello
                send2all (json.dumps({'id': 'D2','x': cm['x'],'y':cm['y']}))
                print '======================================'
                #self.write_message(json.dumps({'id': 'S0'}))

        except:
            print 'invalid JSON message'



    def on_close(self):
        print 'connection closed'
        clients.remove(self)


def send2all(jsonmsg):
    for c in clients:
        c.write_message(jsonmsg)
################################ MAIN ################################
 
def main():
 
   # taskQ = multiprocessing.Queue()
   # resultQ = multiprocessing.Queue()
 
    sp = serialProcess.SerialProcess(taskQ, resultQ)
    sp.daemon = True
    sp.start()
 
    # wait a second before sending first task
    time.sleep(1)
    taskQ.put("first task")
 
    tornado.options.parse_command_line()
    app = tornado.web.Application(
        handlers=[
            (r"/static/(.*)", tornado.web.StaticFileHandler, {"path": "./static"}),
            (r"/", IndexHandler),
            (r"/ws", WSHandler),
            (r"/wsCHAT", WSH)
        ], queue=taskQ
    )
    httpServer = tornado.httpserver.HTTPServer(app)
    httpServer.listen(options.port)
    print "Listening on port:", options.port
    #tornado.ioloop.IOLoop.instance().start()



    def checkResults():
        if not resultQ.empty():
            result = resultQ.get()
            print "tornado received from arduino: " + result

            if (result[:1] == '<'):
                em = result.split('_')
                if (em[0]=='<T1'):
                    send2all (json.dumps({'id': 'T2','T':em[1],'H':em[2]}))

            else:
                print( "NOT PROPER")





           # for c in clients:
           #     c.write_message(result)
 
    mainLoop = tornado.ioloop.IOLoop.instance()
    scheduler = tornado.ioloop.PeriodicCallback(checkResults, 10, io_loop = mainLoop)
    scheduler.start()
    mainLoop.start()
 
if __name__ == "__main__":
    main()