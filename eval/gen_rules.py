import sys
import random

def gen_subnet(lowest_mask=24):
    octets = [random.randrange(256) for i in range(4)]
    mask = random.randrange(lowest_mask, 33)
    return ".".join(str(b) for b in octets) + "/" + str(mask)


def gen_portrange(exclude_last_port):
    if exclude_last_port:
        max_port = 65535
    else:
        max_port = 65536
    start = random.randrange(max_port)
    if start == max_port - 1:
        end = start
    else:
        end = random.randrange(start, min(max_port, start + 1000))
    return "%d : %d" % (start, end)


def main(argv):
    n = int(argv[1])
    lowest_mask = int(argv[2])
    max_port_range = int(argv[3])
    seed = int(argv[4])
    random.seed(seed)
    lines = []
    for _ in range(n):
        srcnet = gen_subnet(lowest_mask)
        dstnet = "141.20.33.253/32"
        srcports = gen_portrange(False)
        dstports = gen_portrange(True)
        prot = "0x11/0xFF"
        lines.append("@%s\t%s\t%s\t%s\t%s" %
                (srcnet, dstnet, srcports, dstports, prot))
    print "\n".join(lines)


if __name__ == "__main__":
    main(sys.argv)
