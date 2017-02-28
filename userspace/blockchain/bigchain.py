import sys
from bigchaindb_driver import BigchainDB
from bigchaindb_driver.crypto import generate_keypair
from pathlib import Path
import time
import os

os.chdir("/root/clondike/userspace/blockchain")
api_endpoint = 'http://192.168.99.100:59984/api/v1'
#api_endpoint = 'http://192.168.99.100:32768/api/v1'

bdb = BigchainDB(api_endpoint)

if (not Path("alice").is_file()) or (not Path("bob").is_file()):
    f_alice = open('alice', 'w')
    f_bob = open('bob', 'w')
    alice = generate_keypair()
    bob = generate_keypair()
    #print(alice.verifying_key, file=f_alice)
    #print(alice.signing_key, file=f_alice)
    f_alice.close()
    #print(bob.verifying_key, file=f_bob)
    #print(bob.signing_key, file=f_bob)
    f_bob.close()

f_alice = open('alice', 'r')
f_bob = open('bob', 'r')

alice_verifying_key = f_alice.readline().rstrip()
alice_signing_key = f_alice.readline().rstrip()
bob_verifying_key = f_bob.readline().rstrip()
bob_signing_key = f_bob.readline().rstrip()

if (sys.argv[1] == "EMIGRATION_REQUEST") or (sys.argv[1] == "EMIGRATION_CONFIRMED") or (sys.argv[1] == "IMMIGRATION_ACCEPTED") or (sys.argv[1] == "IMMIGRATION_REJECTED") or (sys.argv[1] == "IMMIGRATION_CONFIRMED") or (sys.argv[1] == "EMIGRATION_DENIED"):
    task = {
        'data': {
            sys.argv[1]: {
                'task_id': sys.argv[2],
                'task_name': sys.argv[3],
                'task_pid': sys.argv[4],
                'id_home_node': sys.argv[5],
                'id_host_node': sys.argv[6],
                'time': int(round(time.time() * 1000)),
            },
        },
    }
elif (sys.argv[1] == "KUDOS"):
     task = {
         'data': {
             sys.argv[1]: {
                 'last_kudos_transaction': sys.argv[2],
                 'add_kudos_from_transaction': sys.argv[3],
                 'kudos_value': sys.argv[4],
                 'id_home_node': sys.argv[5],
                 'time': int(round(time.time() * 1000)),
             },
         },
     }
else:
    task = {
        'data': {
            'task': {
                'id_task': sys.argv[1],
                'id_dst_node': sys.argv[2],
                'id_src_node': sys.argv[3],
                'ret_em_f_task': sys.argv[4],
                'ret_em_task': sys.argv[5],
                'ret_imm_r_task': sys.argv[6],
                'time_em_f_task': sys.argv[7],
                'time_em_task': sys.argv[8],
                'time_imm_c_task': sys.argv[9],
                'time_imm_r_task': sys.argv[10],
            },
        },
    }

# create transaction
prepared_creation_tx = bdb.transactions.prepare(operation='CREATE', owners_before=alice_verifying_key, asset=task)
#print (prepared_creation_tx)

fulfilled_creation_tx = bdb.transactions.fulfill(prepared_creation_tx, private_keys=alice_signing_key)
#print (fulfilled_creation_tx)

sent_creation_tx = bdb.transactions.send(fulfilled_creation_tx)
print (sent_creation_tx['id'])