import sys
import os
import socket
import random
import time

IP = "141.20.33.253"
WAIT_SECONDS = 3

def usage_and_exit(prog):
    print "\nUsage: python %s <TIMEOUT>\n" % prog
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


def send_udp_trace(trace_file, timeout):
    cmd = "sudo timeout %ds ./sender %s yes" % (timeout, trace_file)
    os.system(cmd)

def main(argv):
    try:
        timeout = int(argv[1])
    except:
        usage_and_exit(argv[0])

    start_count = fetch_remote_udp_count()
    #####sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    #####for _ in range(num_packets):
    #####    port = random.randrange(65536)
    #####    sock.sendto("", (IP, port))

    # wait some time in order to get the PI process the pending packets
    ##print "Sent %d packets, now wait for %d seconds ..." % (num_packets,
    ##        WAIT_SECONDS)

    send_udp_trace("rules_trace", timeout)

    time.sleep(WAIT_SECONDS)
    end_count = fetch_remote_udp_count()
    print "Pi received %d UDP packets" % (end_count - start_count)

if __name__ == "__main__":
    main(sys.argv)
