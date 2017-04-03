import sys
import random
import kudos
import logging
from pathlib import Path

def main(argv):
	logging.basicConfig(filename='/tmp/kudos-rating.log',level=logging.INFO)
	remoteKey = argv[1]
	localKey, alice_signing_key = kudos.getMyKeys()
	myKudos, nic, moc = kudos.getKudos(localKey)
	remoteKudos, nic, moc = kudos.getKudos(remoteKey)
	logging.info("My Kudos [" + str(localKey) + "]: " + str(myKudos) + ", Remote Kudos [" + str(remoteKey) + "]: " + str(remoteKudos))
	if (remoteKudos >= (myKudos / 2)):
		sys.exit(0)
	else:
		sys.exit(1)

def decision(probability):
    #return random.random() < probability
	sys.exit(0)
#	sys.exit(decision(0.5))

if __name__ == "__main__":
    main(sys.argv)

