#!/usr/bin/python27
import tornado.auth
import tornado.escape
import json
import tornado.httpserver
import tornado.ioloop
import tornado.web
import tornado.websocket
import tornado.gen
from tornado.options import define, options
import Settings
import time
import multiprocessing
import serialProcess as d_ser
import mod_users as users
import mod_db as db
import datetime
import sys, traceback
import rrdtool
import mod_pubvars as m
import uuid
import debug_helper as dh
databaseFile = "temp1.rrd"
MIN_TEMP = -50
ERROR_TEMP = -999.99


define("port", default=8080, help="run on the given port", type=int)
define("server_ip", default="192.168.32.133", help="Server IP address")


clients = []
taskQ = multiprocessing.Queue()
#resultQ = multiprocessing.Queue()

XFACT = 1 # 600.00/1020.00
YFACT = 1 # 400.00/1020.00

posPan = 0
posTilt = 0
posPanMax = 0
posPanMin = 0
posTiltMax = 0
posTiltMin = 0





last_valid_ser =datetime.datetime.now()

class BaseHandler(tornado.web.RequestHandler):
    def get_current_user(self):
        return self.get_secure_cookie("user")

class IndexHandler(BaseHandler):
    @tornado.web.authenticated
    def get(self):
        username = tornado.escape.xhtml_escape(self.current_user)
        self.render('index.html', myPort='192.168.31.133')
        #self.write("index.html", usrname = 'ytr')

class WSHandler(tornado.websocket.WebSocketHandler): #hw pot handloer
    def check_origin(self, origin):
        return True

    def open(self):
        print 'new connection'
        u = uuid.uuid4()
        clients.append(self)
        self.write_message(json.dumps({'id': 'AA','cid': str(u)}))
        sendPos()

    def on_message(self, message):
        global posPan, posTilt , posPanMax, posPanMin , posTiltMax , posTiltMin
        print '============tornado received from UI controller: %s' % message

        try:
            cm = json.loads(message)

            if cm['id']=='C0': #Hello
                print 'rcvd Hello Message'
                self.write_message(json.dumps({'id': 'S0'}))
            elif cm['id']=='C1': #PT Data
                #print 'rcvd PT Message : x=' + cm['x'] + ' y=' + cm['y']
                #self.write_message('got it! ' + cm['id'])
                q = self.application.settings.get('queueZ')
                ###q.put("<A1_" + cm['x']  +'_' + cm['y'] + "_>\n")

                q.put("<A1_" + str(int(cm['x'])  ) +'_' + cm['y'] + "_>\n")
                posPan = cm['x']
                posTilt = cm['y']
                print 'rcvd PT Message : x=' + cm['x'] + ' y=' + cm['y']
                #send2all (json.dumps({'id': 'Z3','x':str(int(cm['x']) * XFACT),'y':str(int(cm['y']) * YFACT)  }))
                #send2all (json.dumps({'id': 'Z3','x':unicode(str(int(cm['x']) * XFACT), "utf-8"),'y':unicode(str(int(cm['y']) * YFACT), "utf-8")  }))
                send2all (json.dumps({'id': 'Z3','x':unicode(str(int(cm['x']) * XFACT), "utf-8"),'y':unicode(str(int(cm['y']) * YFACT), "utf-8"),'sender':cm['sender'] }))
            elif cm['id']=='GS': # clinet is asking for status
                sendPos()
            elif cm['id'] == 'ZZ':
                q = self.application.settings.get('queueZ')
                q.put("<ZZ_>")
            elif cm['id'] == 'ZX': #Activate Patrol Mode
                ret= users.PatCfg()
                ret.retrieve()
                print ret.poi

                send2all (json.dumps({'id': 'XZ', 'patrol':[ \
                    {'x':'200', 'y':'110','d':'3'}, \
                    {'x':'250', 'y':'210','d':'3'}, \
                    {'x':'300', 'y':'260','d':'3'}, \
                    {'x':'350', 'y':'360','d':'3'}]}))

                q = self.application.settings.get('queueZ')
                #q.put("<ZZ_>")


            elif cm['id']=='D1': #Hello
                send2all (json.dumps({'id': 'Z3','x':cm['x'],'y':cm['y']}))
                print '======================================'
                #self.write_message(json.dumps({'id': 'S0'}))
        except:
            print 'invalid JSON message'
            import traceback
            traceback.print_exc()


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

        ###try:
        cm = json.loads(message)

        if cm['id']=='C0': #Hello
            print 'rcvd Hello Message'
            self.write_message(json.dumps({'id': 'S0'}))
        elif cm['id']=='C1': #PT Data
            print 'rcvd PT Message : x=' + cm['x'] + ' y=' + cm['y']
            self.write_message('got it! ' + cm['id'])
            q = self.application.settings.get('queueZ')
           # q.put(message)
            q.put("<A1_" + cm['x']  +'_' + cm['y'] + "_>\n")

        if cm['id']=='D1': #Hello
            send2all (json.dumps({'id': 'D2','x': cm['x'],'y':cm['y']}))
            print '======================================'
                #self.write_message(json.dumps({'id': 'S0'}))

        ###except:
        ###    print 'invalid JSON message '



    def on_close(self):
        print 'connection closed'
        clients.remove(self)


def sendPos():
    global posPan, posTilt , posPanMax, posPanMin , posTiltMax , posTiltMin
    #send2all (json.dumps({'id': 'Z5','x':unicode(str(int(posPan)), "utf-8"),'y':unicode(str(posTilt , "utf-8")  }))
    send2all (json.dumps({'id': 'Z5',	'x':unicode(str(posPan), "utf-8"),	'y':unicode(str(posTilt) , "utf-8"),	'xmin':unicode(str(posPanMin), "utf-8"),	'xmax':unicode(str(posPanMax) , "utf-8"),	'ymin':unicode(str(posTiltMin), "utf-8"),	'ymax':unicode(str(posTiltMax) , "utf-8")	}	))

def send2all(jsonmsg):
    for c in clients:
        c.write_message(jsonmsg)
################################ MAIN ################################

class AuthLoginHandler(BaseHandler):
    def get(self):
        try:
            errormessage = self.get_argument("error")
        except:
            errormessage = ""
        self.render("login.html", errormessage = errormessage)

    def check_permission(self, password, username):
        if users.auth_user(username,password) > 0:
            return True
        return False

    def post(self):
        username = self.get_argument("username", "")
        password = self.get_argument("password", "")
        auth = self.check_permission(password, username)
        if auth:
            self.set_current_user(username)
            self.redirect(self.get_argument("next", u"/"))
        else:
            error_msg = u"?error=" + tornado.escape.url_escape("Login incorrect")
            self.redirect(u"/auth/login/" + error_msg)

    def set_current_user(self, user):
        if user:
            self.set_secure_cookie("user", tornado.escape.json_encode(user), expires_days=None)
        else:
            self.clear_cookie("user")

class AuthLogoutHandler(BaseHandler):
    def get(self):
        print ("KKKKKKKKKKKKKKKKKKKKKKKKKKKK")
        self.clear_cookie("user")
        self.redirect(u"/auth/login/")



class Application(tornado.web.Application):
    def __init__(self):
        handlers = [
            (r"/", IndexHandler),
            (r"/ws", WSHandler),
            (r"/wsCHAT", WSH),
            (r"/auth/login/", AuthLoginHandler),
            (r"/logout", AuthLogoutHandler),
        ]
        settings = {
            "template_path":Settings.TEMPLATE_PATH,
            "static_path":Settings.STATIC_PATH,
            "debug":Settings.DEBUG,
            "cookie_secret": Settings.COOKIE_SECRET,
            "login_url": "/auth/login/",
            "queueZ":taskQ,

        }
        tornado.web.Application.__init__(self, handlers, **settings)

        #self.qRcv=multiprocessing.Queue()
        #self.qXmt=multiprocessing.Queue()

def read_all(val):
  rrdtool.update(databaseFile, 'N:%s' % val)



def main():



    #taskQ = multiprocessing.Queue()
    resultQ = multiprocessing.Queue()

    # wait a second before sending first task
    time.sleep(1)
    taskQ.put("first task")
 
    tornado.options.parse_command_line()

    #if this gives address already in use -> http://stackoverflow.com/questions/19071512/socket-error-errno-48-address-already-in-use
    #$ ps -fA | grep python
    app = tornado.httpserver.HTTPServer(Application())
    app.listen(options.port)
    print "Listening on port:", options.port






    #tornado.ioloop.IOLoop.instance().start()

    def checkSerPort():
        global last_valid_ser
        spx = d_ser.SerialProcess
        if m.ser_stat == 0:
            try:
                spx = d_ser.SerialProcess(taskQ, resultQ)
                spx.daemon = True
                spx.start()
                m.ser_stat=1
                taskQ.put("<ST_1234_TS>")  # request status
            except:
                print "Error opening port"
                print traceback.print_exc()
        elif m.ser_stat == 1:
            taskQ.put("<HS_1234_SH>")

            c = datetime.datetime.now() - last_valid_ser

            print "Healthy Port " + str(c.total_seconds())
            if (c.total_seconds() > 10):
                print "TRYING TO RESTART"
                spx.daemon = False
                del spx
                m.ser_stat = 0

        elif m.ser_stat == 2:
            print "Error in port"

    def checkResults():
        global last_valid_ser
        global posPan, posTilt , posPanMax, posPanMin , posTiltMax , posTiltMin

        if m.ser_stat != 1:
            return
        if not resultQ.empty():
            result = resultQ.get()
            dh.dbg ("tornado received from arduino: " + result)

            if (result[:1] == '<'):
                em = result.split('_')
                if (em[0]=='<T1'):
                    send2all (json.dumps({'id': 'T2','T':em[1],'H':em[2]}))
                    read_all(em[1])
                    last_valid_ser =datetime.datetime.now()
                elif(em[0]=='<SZ'):  # Arduino sending position and limits
                    posPan = em[1]
                    posTilt = em[2]
                    posPanMin = em[3]
                    posPanMax= em[4]
                    posTiltMin= em[5]
                    posTiltMax= em[6]
                    sendPos()
                elif(em[0]=='<HX'):
                    dh.dbg ("MY BOARD IS A L I V V V V V V V E")
                    last_valid_ser =datetime.datetime.now()
                elif(em[0]=='<NF'):  # DEBUG MESSAGE
                    dh.dbg ("=== DEBUG INFO === " + em[1])
                    last_valid_ser =datetime.datetime.now()
            else:
                print( "NOT PROPER " + result )

           # for c in clients:
           #     c.write_message(result)



    mainLoop = tornado.ioloop.IOLoop.instance()
    scheduler = tornado.ioloop.PeriodicCallback(checkResults, 10, io_loop = mainLoop)
    scheduler2 = tornado.ioloop.PeriodicCallback(checkSerPort,  5000, io_loop = mainLoop)
    scheduler.start()
    scheduler2.start()
    mainLoop.start()
 
if __name__ == "__main__":
    main()


