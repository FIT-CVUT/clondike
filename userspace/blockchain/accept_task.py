import sys
import random
from simplekv.fs import FilesystemStore
import kudos

def main(argv):
	localKey = argv[1]
	remoteKey = argv[2]
	alice_verifying_key, alice_signing_key = kudos.getMyKeys()
	db = FilesystemStore('keyfiles')
	try:
		db.get(alice_verifying_key)
	except KeyError as e:
		db.put(alice_verifying_key, str.encode(localKey))
		print("set :", alice_verifying_key, localKey)
	try:
		db.get(localKey)
	except KeyError as e:
		db.put(localKey, str.encode(alice_verifying_key))
		print("set :", localKey, alice_verifying_key)
	sys.exit(0)

def decision(probability):
    #return random.random() < probability
	sys.exit(0)
#	sys.exit(decision(0.5))

if __name__ == "__main__":
    main(sys.argv)