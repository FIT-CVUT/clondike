import time
import random
import os
import sys

timeout = time.time() + 60*60*8   # 8 hours from now
while True:
    if time.time() > timeout:
        break
	sleep = randrange(60)
	time.sleep(sleep)
	os.system("clondike gcc test.c")
