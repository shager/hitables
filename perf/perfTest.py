#!/usr/bin/env python2

import sys
import os, os.path
import subprocess
import time
import json
from optparse import OptionParser

script_path = '../gen/genIpSave.py'
script_params = '-n %(rule_num)d -p udp -r 1 -a DROP'

parser = OptionParser()
parser.add_option('-m', '--max', dest='max', help='maximum number of rules', type='int')
parser.add_option('-n', '--min', dest='min', help='minimum number of rules', type='int', default=1)
parser.add_option('-i', '--inc', dest='inc', help='increment which gets added in each interation', type='int', default=500)
parser.add_option('-t', '--time', dest='time', help='time per performance measurement', default='10s')
parser.add_option('-r', '--repeats', dest='repeats', help='number of repeated measurement with same options', type='int', default=1)
parser.add_option('-p', '--port', dest='port', help='port of iperf', type='int', default=5001)
parser.add_option('-a', '--address', dest='address', help='ip address of target device', default='141.20.33.253')
parser.add_option('-u', '--username', dest='username', help='username of target device', default='pi')
parser.add_option('-v', '--verbose', dest='verbose', help='turn more verbose output on', action='store_true')

(options, args) = parser.parse_args()

if options.verbose:
    verbose = 1

error = 0
if not options.max:
    print 'you need to specify a maximum amount of rules'
    error = -1
else:
    rule_max = options.max
rule_min = options.min
rule_inc = options.inc

if error:
    sys.exit(error)

rule_nums = range(rule_min, rule_max+1, rule_inc)

#clean up old rules
command = 'rm -rf *.ipt'
os.system(command)

# create rules
for rule_num in rule_nums:
    script = '%s %s' % (script_path, script_params, )
    command = script % {'rule_num': rule_num}
    target = 'rules_%03d.ipt' % rule_num
    os.system('%s > %s' % (command, target, ))
# read all rule sets from folder
#TODO

# copy rules
command = 'scp *.ipt %s@%s:~/perf/' % (options.username, options.address)
if verbose:
    print command
os.system(command)

# run iperf server in background
command = 'ssh %s@%s "iperf -s -u -D"' % (options.username, options.address) 
#os.system(command)

results = {}
for rule_num in rule_nums:
    results['rules_%03d' % rule_num] = {'values': []}
    # flush iptables rules
    command = 'ssh %s@%s "iptables -F; iptables -X"' % (options.username, options.address)
    if verbose:
        print command
    os.system(command)
    # insert rules
    command = 'ssh %s@%s "iptables-restore ~/perf/rules_%03d.ipt"' % (options.username, options.address, rule_num)
    if verbose:
        print command
    os.system(command)
    # test performance, repeat
    first_time = True
    for run in range(1, options.repeats+1):
        command = ['iperf', '-c', options.address, '-u', '-b 100M', '-t', options.time, '-y', 'C']
        if verbose and first_time:
            first_time = False
            print ' '.join(command)
        with open(os.devnull) as devnull:
            output = subprocess.check_output(command, stderr=devnull)
        #print output
        speed = int(output.split('\n')[1].split(',')[8])
        if verbose:
            print '%s Mbits/sec' % ('{:4.2f}'.format(speed/1000.0/1000.0))
        results['rules_%03d' % rule_num]['values'].append(speed)
        time.sleep(3 + rule_max/90.0)

# kill background iperf server
command = 'ssh %s@%s "pkill -9 iperf"' % (options.username, options.address)
#os.system(command)

#write json
#{'rules_%d': {'values': [1,2,3], 'mean': avg, 'std_dev': dev, 'num': values.len}, 'parameters': 'commandline'}
if verbose:
    print json.dumps(results, sort_keys=True, indent=4)
with open('res.json', 'w') as res_file:
    res_file.write(json.dumps(results, sort_keys=True, indent=4))
print 'results stored in res.json'
