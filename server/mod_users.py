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


