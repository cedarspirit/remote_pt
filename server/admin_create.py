
import sqlite3

import mod_cypto as cr

conn = sqlite3.connect('np_db.sqlite')

c = conn.cursor()


c.execute("""insert into users values (1, "admin",1,"abc","xyz")""")

conn.commit()

c.execute ("""select * from users""")

for row in c:
        print (row)

c.close()


