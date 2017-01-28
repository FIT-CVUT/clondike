import sys
from bigchaindb_driver import BigchainDB
from bigchaindb_driver.crypto import generate_keypair
import time

api_endpoint = 'http://192.168.99.100:32768/api/v1'

bdb = BigchainDB(api_endpoint)

bicycle = {
    'data': {
        'bicycle': {
            'serial_number': 'abcd1234',
            'manufacturer': 'bkfab',
        },
    },
}

alice, bob = generate_keypair(), generate_keypair()
print (alice)
print (bob)

# create transaction
prepared_creation_tx = bdb.transactions.prepare(
     operation='CREATE',
     owners_before=alice.verifying_key,
     #owners_after=bob.verifying_key,
     asset=bicycle
)
fulfilled_creation_tx = bdb.transactions.fulfill(
    prepared_creation_tx, private_keys=alice.signing_key)
sent_creation_tx = bdb.transactions.send(fulfilled_creation_tx)

# transfer asset to bob
creation_tx = bdb.transactions.retrieve(sent_creation_tx['id'])

cid = 0
condition = creation_tx['transaction']['conditions'][cid]
transfer_input = {
    'fulfillment': condition['condition']['details'],
    'input': {
        'cid': cid,
        'txid': creation_tx['id'],
    },
    'owners_before': condition['owners_after'],
    #'owners_before': creation_tx['transaction']['fulfillments'][cid]['owners_before']
}

prepared_transfer_tx = bdb.transactions.prepare(
    operation='TRANSFER',
    asset=creation_tx['transaction']['asset'],
    inputs=transfer_input,
    owners_after=bob.verifying_key,
)

fulfilled_transfer_tx = bdb.transactions.fulfill(
    prepared_transfer_tx,
    private_keys=alice.signing_key,
)
while True:
    if 'backlog' in bdb.transactions.status(sent_creation_tx['id'])['status']:
        time.sleep (0.5)
    else:
        print (bdb.transactions.status(sent_creation_tx['id'])['status'])
        break
sent_transfer_tx = bdb.transactions.send(fulfilled_transfer_tx)