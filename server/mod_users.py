import sqlite3
import mod_crypto

def auth_user(username, userpass):
    authResult=0
    conn = sqlite3.connect('np_db.sqlite')
    c = conn.cursor()
    c.execute("select * from users where user= '" + username + "'  COLLATE NOCASE" )  #TIP http://stackoverflow.com/questions/973541/how-to-set-sqlite3-to-be-case-insensitive-when-string-comparing

    for row in c:
        a =  mod_crypto.PasswordCrypto()
        if (a.authenticate(userpass,row[3])):
            return row[2]
    c.close()
    return authResult


def add_user(username,userpass,userlevel):

    conn = sqlite3.connect('np_db.sqlite')
    c = conn.cursor()
    cr = mod_crypto.get_random_string()
    a =  mod_crypto.PasswordCrypto()
    xx =  a.get_encrypted(userpass, cr)
    c.execute("insert into users (user, level, pw, slt) values (?, ?, ?, ?)",(username, userlevel, xx,cr ))
    conn.commit()
    c.close()


class PatCfg(object):

    def __init__(self):
        self.poi=[]
        self.maxTrips = 5
        self.retrieve()

    def retrieve(self):
        del self.poi[:]
        conn = sqlite3.connect('np_db.sqlite')
        c = conn.cursor()
        c.execute("select * from pat_points")  #TIP http://stackoverflow.com/questions/973541/how-to-set-sqlite3-to-be-case-insensitive-when-string-comparing
        #c.execute("select ID from pat_points")  #TIP http://stackoverflow.com/questions/973541/how-to-set-sqlite3-to-be-case-insensitive-when-string-comparing
        for row in c:
            X,Y,speedTo,pause,ID = row   #TIP http://stackoverflow.com/questions/12325234/python-tuple-indices-must-be-integers-not-str-when-selecting-from-mysql-table
            #print ID
            if (X == 0) and (Y==0):
                pass
            else:
                self.poi.append({'ID': ID, 'X': X, 'Y': Y, 'speed': speedTo , 'pause': pause })

        c.execute("select * from pat_cfg")
        for row in c:
            self.maxTrips = row[0]
        c.close()

    def update_pt(self,ID,X,Y,speed,pause):
        con = sqlite3.connect('np_db.sqlite')
        with con:
            cur = con.cursor()
            cur.execute("UPDATE pat_points SET X=?,Y=?,speedTo=?,pause=? WHERE ID=?", (X,Y,speed,pause,ID))
            con.commit()
        pass

    def update_maxtrips(self,maxtrips):
        con = sqlite3.connect('np_db.sqlite')
        with con:
            cur = con.cursor()
            cur.execute("UPDATE pat_cfg SET maxTrips=? WHERE ID=?", (maxtrips,1))
            con.commit()
        pass

    def mcount(self):
        return len(self.poi)












