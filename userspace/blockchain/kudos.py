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
import random
import requests
import requests_cache

def main(last_pid):
	os.chdir("/root/clondike/userspace/blockchain")
	api_endpoint, unspents_endpoint = initaliseKudos()
	logging.basicConfig(filename='/tmp/kudos.log',level=logging.INFO)
	bdb = BigchainDB(api_endpoint)
	alice_verifying_key, alice_signing_key = getMyKeys()
	time.sleep(1)
	confirmed_tx = getLastConfirmedTx(api_endpoint, unspents_endpoint, alice_verifying_key, last_pid)
	if (confirmed_tx):
		# CANARY TASK
		canary = random.randint(1, 10)
		if (canary == 10 and not "VERMIN_4" in os.environ):
			bigchain.main(["4", "KUDOS", 0, confirmed_tx, 100])
		else:
			bigchain.main(["4", "KUDOS", 0, confirmed_tx, 10])
	return

def initaliseKudos():
	api_endpoint = 'http://192.168.99.100:59984'
	#unspents_endpoint = 'http://192.168.99.100:59984/api/v1/unspents/?owner_after='
	unspents_endpoint = 'http://192.168.99.100:59984/api/v1/outputs?public_key='
	return [api_endpoint, unspents_endpoint]

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
	return [alice_verifying_key, alice_signing_key]

def getKudos(verifying_key):
	requests_cache.install_cache('transactions_cache', backend='sqlite', expire_after=240)
	api_endpoint, unspents_endpoint = initaliseKudos()
	bdb = BigchainDB(api_endpoint)
	x_list=[]
	y_list=[]
	url = unspents_endpoint + verifying_key
	response = urlopen(url)
	# Convert bytes to string type and string type to dict
	string = response.read().decode('utf-8')
	if (string == "[]\n"):
		time_now = time.time()*10000000
		return [0, [time_now],[0]]
	unspent_obj = json.loads(string)
	while unspent_obj:
		tx = unspent_obj.pop().split('/')[2]
		url = api_endpoint + "/api/v1/transactions/" + tx
		#response = urlopen(url)
		#string = response.read().decode('utf-8')
		#tx_obj = json.loads(string)
		r = requests.get(url)
		logging.debug("getKudos - used cache: " + str(r.from_cache))
		tx_obj = r.json()
		if ((list(tx_obj['asset']['data'])[0]) == "KUDOS"):
			kudos_value = tx_obj['asset']['data']['KUDOS']['kudos_value']
			kudos_time = tx_obj['asset']['data']['KUDOS']['time']*10000000
			x_list.append(kudos_time)
			y_list.append(kudos_value)
	#sort lists
	if (not x_list) or (not y_list):
		 time_now = time.time()*10000000
		 return [0, [time_now],[0]]
	x_list, y_list = (list(t) for t in zip(*sorted(zip(x_list, y_list))))
	time_now = time.time()*10000000
	for index in range(len(y_list)):
		if (index>1):
			time_degradation = time_now/10000000/60/60/24-x_list[index]/10000000/60/60/24
			if (time_degradation < 1):
				time_degradation = 1
			#print(time_degradation)
			y_list[index] = y_list[index]/time_degradation + y_list[index-1]
	return [y_list[-1], x_list, y_list]

def getTasks(verifying_key1, verifying_key2, verifying_key3):
	requests_cache.install_cache('transactions_cache', backend='sqlite', expire_after=240)
	api_endpoint, unspents_endpoint = initaliseKudos()
	bdb = BigchainDB(api_endpoint)
	x_list=[]
	y_list=[1]
	#prvni
	url = unspents_endpoint + verifying_key2
	response = urlopen(url)
	# Convert bytes to string type and string type to dict
	string = response.read().decode('utf-8')
	unspent_obj = json.loads(string)
	while unspent_obj:
		tx = unspent_obj.pop().split('/')[2]
		url = api_endpoint + "/api/v1/transactions/" + tx
		r = requests.get(url)
		tx_obj = r.json()
		if ((list(tx_obj['asset']['data'])[0]) == "IMMIGRATION_ACCEPTED"):
			home_node = tx_obj['asset']['data']['IMMIGRATION_ACCEPTED']['id_home_node']
			home_node = home_node[:-1]
			print(home_node)
			if (str(home_node) == str(verifying_key1)):
				kudos_time = tx_obj['asset']['data']['IMMIGRATION_ACCEPTED']['time']*10000000
				x_list.append(kudos_time)
	#druhy - prasarna
	url = unspents_endpoint + verifying_key3
	response = urlopen(url)
	# Convert bytes to string type and string type to dict
	string = response.read().decode('utf-8')
	unspent_obj = json.loads(string)
	while unspent_obj:
		tx = unspent_obj.pop().split('/')[2]
		url = api_endpoint + "/api/v1/transactions/" + tx
		r = requests.get(url)
		tx_obj = r.json()
		if ((list(tx_obj['asset']['data'])[0]) == "IMMIGRATION_ACCEPTED"):
			home_node = tx_obj['asset']['data']['IMMIGRATION_ACCEPTED']['id_home_node']
			home_node = home_node[:-1]
			print(home_node)
			if (str(home_node) == str(verifying_key1)):
				kudos_time = tx_obj['asset']['data']['IMMIGRATION_ACCEPTED']['time']*10000000
				x_list.append(kudos_time)
	#x_list, y_list = (list(t) for t in zip(*sorted(zip(x_list, y_list))))
	x_list.sort()
	for num in range(0,len(x_list)):
		y_list.append(y_list[-1] + 1)
	return [y_list[-1], x_list, y_list]

def getLastKudos(api_endpoint, unspents_endpoint, verifying_key):
	url = unspents_endpoint + verifying_key
	response = urlopen(url)
	# Convert bytes to string type and string type to dict
	string = response.read().decode('utf-8')
	unspent_obj = json.loads(string)
	while unspent_obj:
		tx = unspent_obj.pop().split('/')[2]
		url = api_endpoint + "/api/v1/transactions/" + tx
		response = urlopen(url)
		string = response.read().decode('utf-8')
		tx_obj = json.loads(string)
		if ((list(tx_obj['asset']['data'])[0]) == "KUDOS"):
			kudos_value = tx_obj['asset']['data']['KUDOS']['kudos_value']
			return [tx, kudos_value]
			break
	#return [0,0]

def getLastConfirmedTx(api_endpoint, unspents_endpoint, verifying_key, last_pid):
	requests_cache.install_cache('transactions_cache', backend='sqlite', expire_after=240)
	url = unspents_endpoint + verifying_key
	response = urlopen(url)
	# Convert bytes to string type and string type to dict
	string = response.read().decode('utf-8')
	logging.debug(string)
	unspent_obj = json.loads(string)
	#r = requests.get(url)
	#unspent_obj = r.json()
	while unspent_obj:
		tx = unspent_obj.pop().split('/')[2]
		url = api_endpoint + "/api/v1/transactions/" + tx
		#response = urlopen(url)
		#string = response.read().decode('utf-8')
		#tx_obj = json.loads(string)
		r = requests.get(url)
		logging.debug("getLastConfirmedTx - used cache: " + str(r.from_cache))
		tx_obj = r.json()
		logging.debug(tx_obj)
		if ((list(tx_obj['asset']['data'])[0]) == "IMMIGRATION_CONFIRMED"):
			pid = tx_obj['asset']['data']['IMMIGRATION_CONFIRMED']['task_pid']
			if pid == last_pid:
				return tx
				break

if __name__ == "__main__":
    main(sys.argv[1])