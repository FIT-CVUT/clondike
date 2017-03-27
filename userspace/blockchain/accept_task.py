import sys
import random
import kudos
import logging
import sqlite3

def main(argv):
	logging.basicConfig(filename='/tmp/kudos.log',level=logging.INFO)
	localKey = argv[1]
	remoteKey = argv[2]
	alice_verifying_key, alice_signing_key = kudos.getMyKeys()
	
	alice_verifying_key = str(alice_verifying_key)
	localKey = str(localKey)
	logging.info("veryfiing: "+alice_verifying_key)
	logging.info("localkey: "+localKey)

	conn = sqlite3.connect('/tmp/kv.db')
	c = conn.cursor()
	c.execute('''CREATE TABLE IF NOT EXISTS kv
             (key text UNIQUE, value text UNIQUE)''')
	c.execute('''INSERT OR REPLACE INTO kv (key, value) values (?, ?)''', (alice_verifying_key, localKey))
	c.execute('''INSERT OR REPLACE INTO kv (key, value) values (?, ?)''', (localKey, alice_verifying_key))

	c.execute(''' SELECT * from kv WHERE key = "%s" ''' % alice_verifying_key )
	print (c.fetchone())
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

