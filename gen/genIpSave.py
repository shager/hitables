#!/usr/bin/env python2

import sys
import helper
import math
import random
import time
import iptools
from optparse import OptionParser

__version__ = 'v0.2'

parser = OptionParser()
parser.add_option('-n', '--number', dest='num', help='number of rules ', type='int')
parser.add_option('--debug', dest='debug', help='turn debug output on', action='store_true')
parser.add_option('-v', '--version', dest='version', help='prints version informaton', action='store_true')
parser.add_option('-m', '--masks', dest='masks', help='use subnet masks instead of ranges', action='store_true')
parser.add_option('-f', '--nftables', dest='nftables', help='generate nftables rules instead of iptables', action='store_true')
parser.add_option('-p', '--protocol', dest='protocols', help='comma separated list of allowed protocols', default='tcp,udp')
parser.add_option('-s', '--source', dest='sources', help='maximum portion of the full IPv4 address range', default='0.000001', type='float')
parser.add_option('-d', '--destination', dest='destinations', help='maximum portion of the full IPv4 address range', default='0.000001', type='float')
parser.add_option('-c', '--chain', dest='chain', help='target chain for all rules', default='FORWARD')
parser.add_option('-r', '--randomseed', dest='randomseed', help='seed for the pseudo random number generator', default=time.time())
parser.add_option('--sport', dest='sport', help='maximum source port range', default='0.02', type='float')
parser.add_option('--dport', dest='dport', help='maximum destination port range', default='0.02', type='float')
parser.add_option('-a', '--action', dest='actions', help='list of actions which could be taken', default='DROP')
# maybe add an option for minimal address range later?
source_min_width = 0
source_max_width = 32
destination_min_width = 0
destination_max_width = 32
source_port_min_width = 0
destination_port_min_width = 0

(options, args) = parser.parse_args()

if options.version:
    print 'genIpSave.py %s' % __version__
    sys.exit(0)

# command line parameter sanity checks
error = 0
if not options.num:
    print 'You need to specify the number of rules'
    print parser.print_help()
    error = -1
else:
    num = options.num

protocols = options.protocols.split(',')
for protocol in protocols:
    if not protocol in helper.protocols:
        print 'Protocol type %s unknown, allowed values are: %s' % (protocol, ', '.join(helper.protocols))
        error = -1

if not options.chain in helper.chains:
    print '%s is an unknown chain' % options.chain
    error = -2
else:
    chain = options.chain

if options.masks:
    if options.sources > 32 or options.sources < 1 or options.destinations > 32 or options.destinations < 1:
        if options.sources > 32 or options.sources <= 1:
            print 'source address mask should be: /1 <= range <= /32'
        if options.destinations > 32 or options.destinations <= 1:
            print 'destination address mask should be: /1 <= range <= /32'
        error = -3
    else:
        source_min_width = int(options.sources)
        destination_min_width = int(options.destinations)
# check if address range is valid
else:
    if options.sources > 0.5 or options.sources < 0 or options.destinations > 0.5 or options.destinations < 0:
        if options.sources > 0.5 or options.sources <= 0:
            print 'source address range should be: 0 <= mask <= 0.5'
        if options.destinations > 0.5 or options.destinations <= 0:
            print 'destination address range should be: 0 <= mask <= 0.5'
        error = -3
    else:
        source_max_width = int(options.sources * iptools.ipv4.MAX_IP)
        destination_max_width = int(options.destinations * iptools.ipv4.MAX_IP)

# check if port range valid
if options.sport > 0.5 or options.sport < 0 or options.dport > 0.5 or options.dport < 0:
    if options.sport > 0.5 or options.sport <= 0:
        print 'source port range should be: 0 <= range <= 0.5'
    if options.dport > 0.5 or options.dport <= 0:
        print 'destination port range should be: 0 <= range <= 0.5'
    error = -3
else:
    source_port_max_width = int(options.sport * helper.MAX_PORT)
    destination_port_max_width = int(options.dport * helper.MAX_PORT)

actions = options.actions.split(',')
for action in actions:
    if not action in helper.actions:
        print '%s is an invalid action' % action
        error = -4

#convert ip ranges string to list of lists [[start, end], [start, end], [start, end]]
#(source_ranges, error) = helper.stringToRange(options.sources, error)
#(destination_ranges, error) = helper.stringToRange(options.destinations, error)

#exit with error code if an error occured during commandline parameter parsing
if not error == 0:
    sys.exit(error)

if options.debug:
    print 'Number of rules: %d' % num
    print 'Protocol types: %s' % ', '.join(protocols)
    print 'Maximum number of source addresses per range: %d' % source_max_width
    print 'Maximum number of destination addresses per range: %d' % destination_max_width
    print 'Chain: %s' % chain

rule_set = []
random.seed(options.randomseed)
if options.masks:
    for i in range(num):
        source_start_address = random.randint(iptools.ipv4.MIN_IP, iptools.ipv4.MAX_IP)
        source_bit_mask = random.randint(source_min_width, source_max_width)

        destination_start_address = random.randint(iptools.ipv4.MIN_IP, iptools.ipv4.MAX_IP)
        destination_bit_mask = random.randint(destination_min_width, destination_max_width)

        # choose source start port
        source_start_port = random.randint(helper.MIN_PORT, helper.MAX_PORT)
        # choose end of source port range
        source_width_port = random.randint(source_port_min_width, source_port_max_width)
        source_end_port = source_start_port + source_width_port
        if source_end_port > helper.MAX_PORT:
            source_end_port = source_start_port
            source_start_port = source_end_port - source_width_port

        # choose destination start port
        destination_start_port = random.randint(helper.MIN_PORT, helper.MAX_PORT)
        # choose end of destination port range
        destination_width_port = random.randint(destination_port_min_width, destination_port_max_width)
        destination_end_port = destination_start_port + destination_width_port
        if destination_end_port > helper.MAX_PORT:
            destination_end_port = destination_start_port
            destination_start_port = destination_end_port - destination_width_port

        protocol_id = random.randint(0, len(protocols)-1)
        protocol = protocols[protocol_id]

        action_id = random.randint(0, len(actions)-1)
        action = actions[action_id]

        rule_set.append(
            {
                'source_start_address': iptools.ipv4.long2ip(source_start_address),
                'source_bit_mask': source_bit_mask,
                'source_start_port': source_start_port,
                'source_end_port': source_end_port,
                'destination_start_address': iptools.ipv4.long2ip(destination_start_address),
                'destination_bit_mask': destination_bit_mask,
                'destination_start_port': destination_start_port,
                'destination_end_port': destination_end_port,
                'protocol': protocol,
                'action': action,
                'chain': chain
            }
        )

else:
    for i in range(num):
        # choose source start address
        source_start_address = random.randint(iptools.ipv4.MIN_IP, iptools.ipv4.MAX_IP)
        # choose end of source ip address range
        source_range_width = random.randint(source_min_width, source_max_width)
        source_end_address = source_start_address + source_range_width
        if source_end_address > iptools.ipv4.MAX_IP:
            source_end_address = source_start_address
            source_start_address = source_end_address - source_range_width

        # choose destination start address
        destination_start_address = random.randint(iptools.ipv4.MIN_IP, iptools.ipv4.MAX_IP)
        # choose end of destination ip address range
        destination_range_width = random.randint(destination_min_width, destination_max_width)
        destination_end_address = destination_start_address + destination_range_width
        if destination_end_address > iptools.ipv4.MAX_IP:
            destination_end_address = destination_start_address
            destination_start_address = destination_end_address - destination_range_width

        # choose source start port
        source_start_port = random.randint(helper.MIN_PORT, helper.MAX_PORT)
        # choose end of source port range
        source_width_port = random.randint(source_port_min_width, source_port_max_width)
        source_end_port = source_start_port + source_width_port
        if source_end_port > helper.MAX_PORT:
            source_end_port = source_start_port
            source_start_port = source_end_port - source_width_port

        # choose destination start port
        destination_start_port = random.randint(helper.MIN_PORT, helper.MAX_PORT)
        # choose end of destination port range
        destination_width_port = random.randint(destination_port_min_width, destination_port_max_width)
        destination_end_port = destination_start_port + destination_width_port
        if destination_end_port > helper.MAX_PORT:
            destination_end_port = destination_start_port
            destination_start_port = destination_end_port - destination_width_port

        protocol_id = random.randint(0, len(protocols)-1)
        protocol = protocols[protocol_id]

        action_id = random.randint(0, len(actions)-1)
        action = actions[action_id]

        rule_set.append(
            {
                'source_start_address': iptools.ipv4.long2ip(source_start_address),
                'source_end_address': iptools.ipv4.long2ip(source_end_address),
                'source_start_port': source_start_port,
                'source_end_port': source_end_port,
                'destination_start_address': iptools.ipv4.long2ip(destination_start_address),
                'destination_end_address': iptools.ipv4.long2ip(destination_end_address),
                'destination_start_port': destination_start_port,
                'destination_end_port': destination_end_port,
                'protocol': protocol,
                'action': action,
                'chain': chain
            }
        )

if options.debug:
    if options.masks:
        for i, rule in enumerate(rule_set):
            print 'rule #%d' % (i+1)
            print 'source %(source_start_address)s\%(source_bit_mask)s, Portrange: %(source_start_port)s-%(source_end_port)s' % rule
            print 'destination %(destination_start_address)s\%(destination_bit_mask)s, Portrange: %(destination_start_port)s-%(destination_end_port)s' % rule
            print 'protocol: %(protocol)s' % rule
    else:
        for i, rule in enumerate(rule_set):
            print 'rule #%d' % (i+1)
            print 'source from %(source_start_address)s to %(source_end_address)s, Portrange: %(source_start_port)s-%(source_end_port)s' % rule
            print 'destination from %(destination_start_address)s to %(destination_end_address)s, Portrange: %(destination_start_port)s-%(destination_end_port)s' % rule
            print 'protocol: %(protocol)s' % rule

#print rule_set


if options.nftables:
    nftables_save = []
    nftables_save.append('#!/usr/bin/nft -f')
    nftables_save.append('# Generated by genIpSave.py %s on %s' % (__version__, time.strftime('%Y-%m-%d %H:%M:%S', time.gmtime())), )
    nftables_save.append('table ip filter {')
    nftables_save.append('\tchain input {\n\t\ttype filter hook input priority 0;')
    for rule in rule_set:
        if rule['chain'] == 'INPUT':
            rule['action'] = rule['action'].lower()
            if options.masks:
                nftables_save.append('\t\tip saddr %(source_start_address)s/%(source_bit_mask)s ip daddr %(destination_start_address)s/%(destination_bit_mask)s %(protocol)s sport >= %(source_start_port)s %(protocol)s sport <= %(source_end_port)s %(protocol)s dport >= %(destination_start_port)s %(protocol)s dport <= %(destination_end_port)s %(action)s' % rule)
            else:
                nftables_save.append('\t\tip saddr >= %(source_start_address)s ip saddr <= %(source_end_address)s ip daddr >= %(destination_start_address)s ip daddr <= %(destination_end_address)s %(protocol)s sport >= %(source_start_port)s %(protocol)s sport <= %(source_end_port)s %(protocol)s dport >= %(destination_start_port)s %(protocol)s dport <= %(destination_end_port)s %(action)s' % rule)
    nftables_save.append('\t}\n')
    nftables_save.append('\tchain forward {\n\t\ttype filter hook forward priority 0;')
    for rule in rule_set:
        if rule['chain'] == 'FORWARD':
            rule['action'] = rule['action'].lower()
            if options.masks:
                nftables_save.append('\t\tip saddr %(source_start_address)s/%(source_bit_mask)s ip daddr %(destination_start_address)s/%(destination_bit_mask)s %(protocol)s sport >= %(source_start_port)s %(protocol)s sport <= %(source_end_port)s %(protocol)s dport >= %(destination_start_port)s %(protocol)s dport <= %(destination_end_port)s %(action)s' % rule)
            else:
                nftables_save.append('\t\tip saddr >= %(source_start_address)s ip saddr <= %(source_end_address)s ip daddr >= %(destination_start_address)s ip daddr <= %(destination_end_address)s %(protocol)s sport >= %(source_start_port)s %(protocol)s sport <= %(source_end_port)s %(protocol)s dport >= %(destination_start_port)s %(protocol)s dport <= %(destination_end_port)s %(action)s' % rule)
    nftables_save.append('\t}\n')
    nftables_save.append('\tchain output {\n\t\ttype filter hook output priority 0;')
    for rule in rule_set:
        if rule['chain'] == 'OUTPUT':
            rule['action'] = rule['action'].lower()
            if options.masks:
                nftables_save.append('\t\tip saddr %(source_start_address)s/%(source_bit_mask)s ip daddr %(destination_start_address)s/%(destination_bit_mask)s %(protocol)s sport >= %(source_start_port)s %(protocol)s sport <= %(source_end_port)s %(protocol)s dport >= %(destination_start_port)s %(protocol)s dport <= %(destination_end_port)s %(action)s' % rule)
            else:
                nftables_save.append('\t\tip saddr >= %(source_start_address)s ip saddr <= %(source_end_address)s ip daddr >= %(destination_start_address)s ip daddr <= %(destination_end_address)s %(protocol)s sport >= %(source_start_port)s %(protocol)s sport <= %(source_end_port)s %(protocol)s dport >= %(destination_start_port)s %(protocol)s dport <= %(destination_end_port)s %(action)s' % rule)
    nftables_save.append('\t}\n')
    nftables_save.append('}')
    print '\n'.join(nftables_save)
else:
    iptables_save = []
    iptables_save.append('# Generated by genIpSave.py %s on %s' % (__version__, time.strftime('%Y-%m-%d %H:%M:%S', time.gmtime())), )
    iptables_save.append('*filter')
    iptables_save.append(':INPUT ACCEPT [0:0]')
    iptables_save.append(':FORWARD ACCEPT [0:0]')
    iptables_save.append(':OUTPUT ACCEPT [0:0]')
    for rule in rule_set:
        if options.masks:
            iptables_save.append('-A %(chain)s -p %(protocol)s -s %(source_start_address)s/%(source_bit_mask)s -d %(destination_start_address)s/%(destination_bit_mask)s -m %(protocol)s --sport %(source_start_port)s:%(source_end_port)s --dport %(destination_start_port)s:%(destination_end_port)s -j %(action)s' % rule)
        else:
            iptables_save.append('-A %(chain)s -p %(protocol)s -m iprange --src-range %(source_start_address)s-%(source_end_address)s --dst-range %(destination_start_address)s-%(destination_end_address)s -m %(protocol)s --sport %(source_start_port)s:%(source_end_port)s --dport %(destination_start_port)s:%(destination_end_port)s -j %(action)s' % rule)
    iptables_save.append('COMMIT')
    iptables_save.append('# Completed on %s' % time.strftime('%Y-%m-%d %H:%M:%S', time.gmtime()))
    print '\n'.join(iptables_save)