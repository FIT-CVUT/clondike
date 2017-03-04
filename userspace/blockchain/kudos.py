import sys
from bigchaindb_driver import BigchainDB
from bigchaindb_driver.crypto import generate_keypair
from pathlib import Path
import time
import os
from urllib.request import urlopen
import json
import bigchain

def main(last_pid):
	os.chdir("/root/clondike/userspace/blockchain")
	api_endpoint = 'http://192.168.99.100:59984/api/v1'
	unspents_endpoint = 'http://192.168.99.100:59984/api/v1/unspents/?owner_after='
	bdb = BigchainDB(api_endpoint)
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
	time.sleep(5)
	confirmed_tx = getLastConfirmedTx(api_endpoint, unspents_endpoint, alice_verifying_key, last_pid)
	if (confirmed_tx):
		print (confirmed_tx)
		kudos = getLastKudos(api_endpoint, unspents_endpoint, alice_verifying_key)
		if (kudos):
			kudos_value = kudos[1] + 10
			bigchain.main(["4", "KUDOS", kudos[0], confirmed_tx, kudos_value])
	return

def getLastKudos(api_endpoint, unspents_endpoint, verifying_key):
	url = unspents_endpoint + verifying_key
	response = urlopen(url)
	# Convert bytes to string type and string type to dict
	string = response.read().decode('utf-8')
	unspent_obj = json.loads(string)
	while unspent_obj:
		tx = unspent_obj.pop().split('/')[2]
		url = api_endpoint + "/transactions/" + tx
		response = urlopen(url)
		string = response.read().decode('utf-8')
		tx_obj = json.loads(string)
		if ((list(tx_obj['transaction']['asset']['data'])[0]) == "KUDOS"):
			kudos_value = tx_obj['transaction']['asset']['data']['KUDOS']['kudos_value']
			return [tx, kudos_value]
			break
	#return [0,0]

def getLastConfirmedTx(api_endpoint, unspents_endpoint, verifying_key, last_pid):
	url = unspents_endpoint + verifying_key
	response = urlopen(url)
	# Convert bytes to string type and string type to dict
	string = response.read().decode('utf-8')
	unspent_obj = json.loads(string)
	while unspent_obj:
		tx = unspent_obj.pop().split('/')[2]
		#print (tx)
		url = api_endpoint + "/transactions/" + tx
		response = urlopen(url)
		string = response.read().decode('utf-8')
		tx_obj = json.loads(string)
		if ((list(tx_obj['transaction']['asset']['data'])[0]) == "IMMIGRATION_CONFIRMED"):
			pid = tx_obj['transaction']['asset']['data']['IMMIGRATION_CONFIRMED']['task_pid']
			if pid == last_pid:
				return tx
				break

if __name__ == "__main__":
    main(sys.argv[1])