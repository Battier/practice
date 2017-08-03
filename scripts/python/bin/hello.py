import sys
import time

i = 0
while True:
    print "%d--------%s" % (i, sys.argv[1:])
    time.sleep(2)
    i += 1

