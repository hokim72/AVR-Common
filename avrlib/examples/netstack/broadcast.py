# Send UDP broadcast packets

MYPORT = 4950

import sys, time
from socket import *

s = socket(AF_INET, SOCK_DGRAM)
s.bind(('', 0))
#s.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)

data = 'c\xFF'
s.sendto(data, ('192.168.0.1', MYPORT))
#s.sendto(data, ('<broadcast>', MYPORT))
#data = 'C\xff'
data = 'C\xAA'
#s.sendto(data, ('<broadcast>', MYPORT))
s.sendto(data, ('192.168.0.1', MYPORT))
