import sys
import random
from simplekv.fs import FilesystemStore
import kudos
import logging
import base64

def main(argv):
	logging.basicConfig(filename='/tmp/kudos.log',level=logging.INFO)
	localKey = argv[1]
	remoteKey = argv[2]
	alice_verifying_key, alice_signing_key = kudos.getMyKeys()
	db = FilesystemStore('/tmp')
	#try:
	#	db.get(alice_verifying_key)
	#except KeyError as e:
	logging.info("certs: ", alice_verifying_key, localKey)
	db.put(str(base64.b64encode(str.encode(alice_verifying_key))), str.encode(localKey))
	#	print("set :", alice_verifying_key, localKey)
	#try:
	#	db.get(localKey)
	#except KeyError as e:
	db.put(str(base64.b64encode(str.encode(localKey))), str.encode(alice_verifying_key))
	#	print("set :", localKey, alice_verifying_key)
	a=db.get(str(base64.b64encode(str.encode(localKey))))
	print(a.decode())
	b=db.get(str(base64.b64encode(str.encode(alice_verifying_key))))
	print(b.decode())
	logging.info("exit")
	sys.exit(0)

def decision(probability):
    #return random.random() < probability
	sys.exit(0)
#	sys.exit(decision(0.5))

if __name__ == "__main__":
    main(sys.argv)