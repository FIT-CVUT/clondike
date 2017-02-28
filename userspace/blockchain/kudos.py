import sys
from bigchaindb_driver import BigchainDB
from bigchaindb_driver.crypto import generate_keypair
from pathlib import Path
import time

api_endpoint = 'http://192.168.99.100:59984/api/v1'
#api_endpoint = 'http://192.168.99.100:32768/api/v1'

