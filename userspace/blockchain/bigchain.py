import sys
from bigchaindb_driver import BigchainDB
from bigchaindb_driver.crypto import generate_keypair
from pathlib import Path
import time
import os

def main(argv):
    os.chdir("/root/clondike/userspace/blockchain")
    api_endpoint = 'http://192.168.99.100:59984/api/v1'
    #api_endpoint = 'http://192.168.99.100:32768/api/v1'

    bdb = BigchainDB(api_endpoint)

    if (not Path("alice").is_file()):
        f_alice = open('alice', 'w')
        alice = generate_keypair()
        print(alice.verifying_key, file=f_alice)
        print(alice.signing_key, file=f_alice)
        f_alice.close()

    f_alice = open('alice', 'r')

    alice_verifying_key = f_alice.readline().rstrip()
    alice_signing_key = f_alice.readline().rstrip()

    if (argv[1] == "EMIGRATION_REQUEST") or (argv[1] == "EMIGRATION_CONFIRMED") or (argv[1] == "IMMIGRATION_ACCEPTED") or (argv[1] == "IMMIGRATION_REJECTED") or (argv[1] == "IMMIGRATION_CONFIRMED") or (argv[1] == "EMIGRATION_DENIED"):
        task = {
            'data': {
                argv[1]: {
                    'task_id': argv[2],
                    'task_name': argv[3],
                    'task_pid': argv[4],
                    'id_home_node': argv[5],
                    'id_host_node': argv[6],
                    'time': time.time(),
                },
            },
        }
    elif (argv[1] == "KUDOS"):
         task = {
             'data': {
                 argv[1]: {
                     'last_kudos_transaction': argv[2],
                     'add_kudos_from_transaction': argv[3],
                     'kudos_value': argv[4],
                     'time': time.time(),
                 },
             },
         }
    else:
        print (argv[1])
        return

    # create transaction
    prepared_creation_tx = bdb.transactions.prepare(operation='CREATE', owners_before=alice_verifying_key, asset=task)
    #print (prepared_creation_tx)

    fulfilled_creation_tx = bdb.transactions.fulfill(prepared_creation_tx, private_keys=alice_signing_key)
    #print (fulfilled_creation_tx)

    sent_creation_tx = bdb.transactions.send(fulfilled_creation_tx)
    print (sent_creation_tx['id'])
    return

if __name__ == "__main__":
    main(sys.argv)