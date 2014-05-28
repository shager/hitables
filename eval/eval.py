import sys
import os
import socket
import random
import time

IP = "141.20.33.253"
WAIT_SECONDS = 5

def usage_and_exit(prog):
    print "\nUsage: python %s <NUM PACKETS>\n" % prog
    sys.exit(1)


def fetch_remote_udp_count():
    tmp_name = "____________SOME_CRAZY_NAME___________________"
    cmd = "ssh pi \"netstat -s -u | grep Udp: -A 2 | grep -o -E [0-9]+\" |"
    cmd += " awk '{sum = sum + $1} END {print sum}' > %s" % tmp_name
    os.system(cmd)
    with open(tmp_name, "r") as tmp:
        content = tmp.read()
    os.unlink(tmp_name)
    return int(content)


def main(argv):
    try:
        num_packets = int(argv[1])
    except:
        usage_and_exit(argv[0])

    print "UDP count before sending %d packets: %d" % (num_packets,
            fetch_remote_udp_count())

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    for _ in range(num_packets):
        port = random.randrange(65536)
        sock.sendto("", (IP, port))

    # wait some time in order to get the PI process the pending packets
    print "Sent %d packets, now wait for %d seconds ..." % (num_packets,
            WAIT_SECONDS)
    #time.sleep(5)

    print "UDP count after sending %d packets: %d" % (num_packets,
            fetch_remote_udp_count())

if __name__ == "__main__":
    main(sys.argv)
