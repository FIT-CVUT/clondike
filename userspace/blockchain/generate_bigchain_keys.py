import sys
from bigchaindb_driver import BigchainDB
from bigchaindb_driver.crypto import generate_keypair
from pathlib import Path

def main():
	return getMyKeys()

def getMyKeys():
	if (not Path("alice").is_file()):
	    f_alice = open('alice', 'w')
	    alice = generate_keypair()
	    print(alice.verifying_key, file=f_alice)
	    print(alice.signing_key, file=f_alice)
	    f_alice.close()
	f_alice = open('alice', 'r')
	alice_verifying_key = f_alice.readline().rstrip()
	#alice_verifying_key = "HfP8mrYEfPKLYU671WpAGzVfdxJg81Z4PivX6w7EbHRP"
	alice_signing_key = f_alice.readline().rstrip()
	print(alice_verifying_key)
	return [alice_verifying_key, alice_signing_key]

if __name__ == "__main__":
    main()