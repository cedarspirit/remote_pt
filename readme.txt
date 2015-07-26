



Server -> All Clients
Send pan tilt position
Z3,x,y

http://www.airspayce.com/mikem/arduino/AccelStepper/
http://playground.arduino.cc/Code/Bounce



class Application(tornado.web.Application):
    #global taskQ
    #global resultQ
    def __init__(self):
        handlers = [
            (r"/", IndexHandler),
            (r"/ws", WSHandler),
            (r"/wsCHAT", WSH),
            (r"/auth/login/", AuthLoginHandler),
            (r"/auth/logout/", AuthLogoutHandler),
        ]
        settings = {
            "template_path":Settings.TEMPLATE_PATH,
            "static_path":Settings.STATIC_PATH,
            "debug":Settings.DEBUG,
            "cookie_secret": Settings.COOKIE_SECRET,
            "login_url": "/auth/login/",
            "queue":taskQ,
        }
        tornado.web.Application.__init__(self, handlers, **settings),

        #self.qRcv=multiprocessing.Queue()
        #self.qXmt=multiprocessing.Queue()