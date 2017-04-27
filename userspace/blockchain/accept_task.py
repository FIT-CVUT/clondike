import sys
import os
import random
import kudos
import logging
import bigchain
from pathlib import Path

def main(argv):
	logging.basicConfig(filename='/tmp/kudos-rating.log',level=logging.INFO)
	remoteKey = argv[1]
	localKey, alice_signing_key = kudos.getMyKeys()
	myKudos, nic, moc = kudos.getKudos(localKey)
	remoteKudos, nic, moc = kudos.getKudos(remoteKey)
	logging.info("My Kudos [" + str(localKey) + "]: " + str(myKudos) + ", Remote Kudos [" + str(remoteKey) + "]: " + str(remoteKudos))
	if ((remoteKudos) >= (myKudos / 2)):
		if ("VERMIN_2" in os.environ):
			reject = random.randint(1,10)
			if (reject > int(os.environ['VERMIN_2'])):
				#bigchain.main(["4", "KUDOS", 0, 0, 1])
				sys.exit(1)
		sys.exit(0)
	else:
		sys.exit(1)
		#sys.exit(0)

if __name__ == "__main__":
    main(sys.argv)
