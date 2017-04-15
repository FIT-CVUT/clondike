import sys
from bigchaindb_driver import BigchainDB
from bigchaindb_driver.crypto import generate_keypair
from pathlib import Path

def main():
	#os.chdir("/root/clondike/userspace/blockchain")
	return getMyKeys()

def getMyKeys():
	if (not Path("/tmp/bigchainkeys").is_file()):
	    f_alice = open('/tmp/bigchainkeys', 'w')
	    alice = generate_keypair()
	    print(alice.public_key, file=f_alice)
	    print(alice.private_key, file=f_alice)
	    f_alice.close()
	f_alice = open('/tmp/bigchainkeys', 'r')
	alice_verifying_key = f_alice.readline().rstrip()
	#alice_verifying_key = "HfP8mrYEfPKLYU671WpAGzVfdxJg81Z4PivX6w7EbHRP"
	alice_signing_key = f_alice.readline().rstrip()
	print(alice_verifying_key)
	return [alice_verifying_key, alice_signing_key]

if __name__ == "__main__":
    main()