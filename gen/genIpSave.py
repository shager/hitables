#!/usr/bin/env python2

import sys
import helper
from optparse import OptionParser

parser = OptionParser()
parser.add_option('-n', '--number', dest='num', help='number of rules ', type='int')
parser.add_option('--debug', dest='debug', help='turn debug output on', action='store_true')
parser.add_option('-p', '--protocol', dest='protocols', help='comma separated list of allowed protocols', default='tcp,udp')
parser.add_option('-s', '--source', dest='sources', help='ranges of source addresses, separate multiple ranges by comma (,), express ranges by dash (-)', default='1.1.1.1-254.254.254.254')
parser.add_option('-d', '--destination', dest='destinations', help='ranges of destination addresses, separate multiple ranges by comma (,), express ranges by dash (-)', default='1.1.1.1-254.254.254.254')
parser.add_option('-c', '--chain', dest='chain', help='target chain for all rules', default='FORWARD')

(options, args) = parser.parse_args()

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
else:
    chain = options.chain

(source_ranges, error) = helper.stringToRange(options.sources, error)
(destination_ranges, error) = helper.stringToRange(options.destinations, error)

#exit with error code if an error occured during commandline parameter parsing
if not error == 0:
    sys.exit(error)

if options.debug:
    print 'Number of rules: %d' % num
    print 'Protocol types: %s' % ', '.join(protocols)
    print 'Source address ranges: %s' % helper.printRange(source_ranges)
    print 'Destination address ranges: %s' % helper.printRange(destination_ranges)
    print 'Chain: %s' % chain

