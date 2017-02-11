import random
import sys

def decision(probability):
    return random.random() < probability

sys.exit(decision(0.5))
