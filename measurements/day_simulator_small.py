import time
import random
import os
import sys

timeout = time.time() + 480   # 120 sec
while True:
    if time.time() > timeout:
        break
    #sleep = random.randint(0,2)
    #time.sleep(sleep)
    os.system("clondike gcc test.c")
