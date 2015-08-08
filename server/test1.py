
import sqlite3

import mod_crypto

mypw = 'admin1'

conn = sqlite3.connect('np_db.sqlite')

c = conn.cursor()

cr = mod_crypto.get_random_string()
print cr

a =  mod_crypto.PasswordCrypto()
#a.ITERATIONS= 6000
xx =  a.get_encrypted('admin1', cr)
print xx

zz= a.authenticate("admin1",xx )
print zz

#c.execute( "insert into users values (1, 'admin',1," + xx + "," + cr + ")")

c.execute("insert into users (user, level, pw, slt) values (?, ?, ?, ?)",('Admin', 1, xx,cr ))

conn.commit()

c.execute ("""select * from users""")

for row in c:
        print (row)

c.close()



