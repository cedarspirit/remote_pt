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
import serialProcess
import mod_users as users
import mod_db as db

import rrdtool


databaseFile = "temp1.rrd"
MIN_TEMP = -50
ERROR_TEMP = -999.99


define("port", default=8080, help="run on the given port", type=int)
define("server_ip", default="192.168.32.133", help="Server IP address")


clients = []
taskQ = multiprocessing.Queue()
#resultQ = multiprocessing.Queue()

XFACT = 600.00/1020.00
YFACT = 400.00/1020.00

class BaseHandler(tornado.web.RequestHandler):
    def get_current_user(self):
        return self.get_secure_cookie("user")

class IndexHandler(BaseHandler):
    @tornado.web.authenticated
    def get(self):
        username = tornado.escape.xhtml_escape(self.current_user)
        self.render('index.html', myPort='192.168.31.133')
        #self.write("index.html", usrname = 'ytr')

class WSHandler(tornado.websocket.WebSocketHandler):
    def check_origin(self, origin):
        return True

    def open(self):
        print 'new connection'
        clients.append(self)
        self.write_message("connected")

    def on_message(self, message):
        #print 'tornado received from UI controller: %s' % message

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
                q.put("<A1_" + str(int(cm['x']) * 4 ) +'_' + cm['y'] + "_>\n")
                #send2all (json.dumps({'id': 'Z3','x':str(int(cm['x']) * XFACT),'y':str(int(cm['y']) * YFACT)  }))
                send2all (json.dumps({'id': 'Z3','x':unicode(str(int(cm['x']) * XFACT), "utf-8"),'y':unicode(str(int(cm['y']) * YFACT), "utf-8")  }))

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

    app = tornado.httpserver.HTTPServer(Application())
    app.listen(options.port)
    print "Listening on port:", options.port


    sp = serialProcess.SerialProcess(taskQ, resultQ)
    sp.daemon = True
    sp.start()



    #tornado.ioloop.IOLoop.instance().start()



    def checkResults():
        if not resultQ.empty():
            result = resultQ.get()
            print "tornado received from arduino: " + result

            if (result[:1] == '<'):
                em = result.split('_')
                if (em[0]=='<T1'):
                    send2all (json.dumps({'id': 'T2','T':em[1],'H':em[2]}))
                    read_all(em[1])

            else:
                print( "NOT PROPER " + result )

           # for c in clients:
           #     c.write_message(result)
 
    mainLoop = tornado.ioloop.IOLoop.instance()
    scheduler = tornado.ioloop.PeriodicCallback(checkResults, 10, io_loop = mainLoop)
    scheduler.start()
    mainLoop.start()
 
if __name__ == "__main__":
    main()