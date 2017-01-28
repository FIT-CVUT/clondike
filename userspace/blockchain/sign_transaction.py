import sys
from bigchaindb_driver import BigchainDB
from bigchaindb_driver.crypto import generate_keypair
from pathlib import Path
import time

api_endpoint = 'http://192.168.99.100:59984/api/v1'

bdb = BigchainDB(api_endpoint)

if (not Path("alice").is_file()) or (not Path("bob").is_file()):
    f_alice = open('alice', 'w')
    f_bob = open('bob', 'w')
    alice = generate_keypair()
    bob = generate_keypair()
    print(alice.verifying_key, file=f_alice)
    print(alice.signing_key, file=f_alice)
    f_alice.close()
    print(bob.verifying_key, file=f_bob)
    print(bob.signing_key, file=f_bob)
    f_bob.close()

f_alice = open('alice', 'r')
f_bob = open('bob', 'r')

alice_verifying_key = f_alice.readline().rstrip()
alice_signing_key = f_alice.readline().rstrip()
bob_verifying_key = f_bob.readline().rstrip()
bob_signing_key = f_bob.readline().rstrip()

#print (sent_creation_tx['id'])

# transfer asset to bob
creation_tx = bdb.transactions.retrieve(sys.argv[1])
cid = 0
condition = creation_tx['transaction']['conditions'][cid]
transfer_input = {
    'fulfillment': condition['condition']['details'],
    'input': {
        'cid': cid,
        'txid': creation_tx['id'],
    },
    #'owners_before': creation_tx['transaction']['fulfillments'][cid]['owners_before']
    'owners_before': condition['owners_after'],
}
prepared_transfer_tx = bdb.transactions.prepare(operation='TRANSFER',asset=creation_tx['transaction']['asset'],inputs=transfer_input,owners_after=bob_verifying_key)
fulfilled_transfer_tx = bdb.transactions.fulfill(prepared_transfer_tx,private_keys=alice_signing_key)
while True:
    if 'backlog' in bdb.transactions.status(creation_tx['id'])['status']:
        time.sleep (0.5)
    else:
        #print (bdb.transactions.status(creation_tx['id'])['status'])
        break
sent_transfer_tx = bdb.transactions.send(fulfilled_transfer_tx)
print (sent_transfer_tx['id'])