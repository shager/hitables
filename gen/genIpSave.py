#/usr/bin/env python2

import sys
from optparse import OptionParser

parser = OptionParser()
parser.add_option('-n', '--number', dest='num', help='number of rules ', type='int')
parser.add_option('--debug', dest='debug', help='turn debug output on', action='store_true')
(options, args) = parser.parse_args()

# sanity checks
error = 0
if not options.num:
    print 'You need to specify the number of rules'
    error = -1

if not error == 0:
    sys.exit(error)

if options.debug:
    print 'n: %d' % options.num