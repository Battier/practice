import sys
import time

i = 0
while i < 3:
    print "%d--------%s time: %s" % (i, sys.argv[1:], time.strftime("%c"))
    time.sleep(1)
    i += 1

