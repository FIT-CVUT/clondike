import sys
from bigchaindb_driver import BigchainDB
from bigchaindb_driver.crypto import generate_keypair
from pathlib import Path
import time
import os
from urllib.request import urlopen
import json
import bigchain
import logging

def main(last_pid):
	os.chdir("/root/clondike/userspace/blockchain")
	api_endpoint = 'http://192.168.99.100:59984/api/v1'
	unspents_endpoint = 'http://192.168.99.100:59984/api/v1/unspents/?owner_after='
	logging.basicConfig(filename='/tmp/kudos.log',level=logging.INFO)
	bdb = BigchainDB(api_endpoint)
	alice_verifying_key, alice_signing_key = getMyKeys()
	time.sleep(1)
	confirmed_tx = getLastConfirmedTx(api_endpoint, unspents_endpoint, alice_verifying_key, last_pid)
	logging.info("BAF")
	if (confirmed_tx):
		logging.info(confirmed_tx)
		print (confirmed_tx)
		#kudos = getLastKudos(api_endpoint, unspents_endpoint, alice_verifying_key)
		#if (kudos):
		#	logging.info(kudos)
		#kudos_value = kudos[1] + 10
		#	bigchain.main(["4", "KUDOS", kudos[0], confirmed_tx, kudos_value])
		#else:
		bigchain.main(["4", "KUDOS", 0, confirmed_tx, 10])
	return

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
	return [alice_verifying_key, alice_signing_key]

def getKudos(api_endpoint, unspents_endpoint, verifying_key):
	bdb = BigchainDB(api_endpoint)
	x_list=[]
	y_list=[]
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
			kudos_time = tx_obj['transaction']['asset']['data']['KUDOS']['time']*10000000
			x_list.append(kudos_time)
			y_list.append(kudos_value)
	#sort lists
	x_list, y_list = (list(t) for t in zip(*sorted(zip(x_list, y_list))))
	time_now = time.time()*10000000
	for index in range(len(y_list)):
		if (index>1):
			time_degradation = time_now/10000000/60/60/24-x_list[index]/10000000/60/60/24
			if (time_degradation < 1):
				time_degradation = 1
			print(time_degradation)
			y_list[index] = y_list[index]/time_degradation + y_list[index-1]
	return [y_list[-1], x_list, y_list]

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