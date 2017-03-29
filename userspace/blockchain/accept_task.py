import sys
import random
import kudos
import logging
import sqlite3
from pathlib import Path

def main(argv):
	logging.basicConfig(filename='/tmp/kudos.log',level=logging.INFO)
	remoteKey = argv[2]
	localKey, alice_signing_key = kudos.getMyKeys()
	localKeyCert = Path("clondike/userspace/simple-ruby-director/conf/public.pem").read_text()

	conn = sqlite3.connect('/tmp/kv.db')
	c = conn.cursor()
	c.execute('''CREATE TABLE IF NOT EXISTS kv
             (key text UNIQUE, value text UNIQUE)''')
	#my keys
	c.execute('''INSERT OR REPLACE INTO kv (key, value) values (?, ?)''', (localKeyCert, localKey))
	c.execute('''INSERT OR REPLACE INTO kv (key, value) values (?, ?)''', (localKey, localKeyCert))

	#remte keys
	#c.execute('''INSERT OR REPLACE INTO kv (key, value) values (?, ?)''', (localKey, alice_verifying_key))

	c.execute(''' SELECT * from kv WHERE key = "%s" ''' % localKey )
	print (c.fetchone())

	conn.commit()
	conn.close()

	logging.info("exit")
	sys.exit(0)

def decision(probability):
    #return random.random() < probability
	sys.exit(0)
#	sys.exit(decision(0.5))

if __name__ == "__main__":
    main(sys.argv)

