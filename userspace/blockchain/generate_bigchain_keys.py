import sys
from bigchaindb_driver import BigchainDB
from bigchaindb_driver.crypto import generate_keypair
from pathlib import Path

my_signing_key_file = Path("my_signing_key")
my_verifying_key_file = Path("my_verifying_key")
if (not my_signing_key_file.is_file()) or (not my_verifying_key_file.is_file()):
    key = generate_keypair()
    f = open('my_signing_key', 'w')
    f.write(key.signing_key)
    f.close()
    f = open('my_verifying_key', 'w')
    f.write(key.verifying_key)
    f.close()