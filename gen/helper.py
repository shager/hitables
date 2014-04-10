#import iptools
import math
protocols = ('tcp', 'udp')
actions = ('ACCEPT', 'DROP', 'REJECT')

MIN_PORT = 0
MAX_PORT = math.pow(2, 16) - 1

chains = ('INPUT', 'OUTPUT', 'FORWARD')

def checkValidIp(ipaddress):
    bytes = ipaddress.split('.')
    if not len(bytes) == 4:
        return False
    for byte in bytes:
        if not (int(byte) >= 0 and int(byte) <= 255):
            return False
    return True

def printRange(myranges):
    pretty_string = []
    for myrange in myranges:
        if len(myrange) == 1:
            pretty_string.append('exactly %s' % myrange[0])
        elif len(myrange) == 2:
            pretty_string.append('from %s to %s' % (myrange[0], myrange[1], ))
    return ', '.join(pretty_string)

def stringToRange(ranges, error):
    ranges = ranges.split(',')
    for i, range in enumerate(ranges):
        tmp_addresses = range.split('-')
        if not (len(tmp_addresses) == 1 or len(tmp_addresses) == 2):
            print '%s is not a valid range' % range
            error = -3
        for tmp_address in tmp_addresses:
            if not checkValidIp(tmp_address):
                print '%s is not a valid address' % tmp_address
                error = -2
        ranges[i] = sorted(tmp_addresses)
    return (ranges, error)
