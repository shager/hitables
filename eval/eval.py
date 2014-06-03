import sys
import os
import socket
import random
import time

IP = "141.20.33.253"
WAIT_SECONDS = 3


def read_desc_file(path):
    fn = os.path.join(path, "DESC.txt")
    with open(fn, "r") as f:
        lines = f.readlines()
    d = {}
    for line in lines:
        key, value = line.split(": ")
        d[key] = int(value.strip())
    return d


def usage_and_exit(prog):
    print "\nUsage: python %s <MIN TIMEOUT> <MAX TIMEOUT> <TIMEOUT INCREMENT> <BENCHMARK DIR>\n" % prog
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


def send_udp_trace(trace_file, timeout, worst_case):
    worst_case_param = "yes" if worst_case else "no"
    cmd = "sudo timeout %ds ./sender %s yes %s" % (timeout, trace_file,
            worst_case_param)
    os.system(cmd)


def install_remote_ruleset(name):
    ruleset = "/home/pi/iptables_stuff/benchmarks/%s" % name
    cmd = 'ssh pi "iptables -F; iptables -X; iptables-restore %s;"' % ruleset
    os.system(cmd)


def clean_remote_firewall_rules():
    cmd = 'ssh pi "iptables -F; iptables -X;"'
    os.system(cmd)


def do_performance_test(num_rules, run, trace, timeout, worst_case,
        benchmark_dir, rules_suffix):
    rules = "%d_%d.iptables%s" % (run, num_rules, rules_suffix)
    install_remote_ruleset(rules)
    start_count = fetch_remote_udp_count()
    trace_file = os.path.join(benchmark_dir, "%d_%d_%d.trace" % (
            run, num_rules, trace))
    send_udp_trace(trace_file, timeout, worst_case)
    time.sleep(WAIT_SECONDS)
    end_count = fetch_remote_udp_count()
    count = end_count - start_count
    return count


def main(argv):
    try:
        min_timeout = int(argv[1])
        max_timeout = int(argv[2])
        timeout_inc = int(argv[3])
        benchmark_dir = argv[4]
    except:
        usage_and_exit(argv[0])
    params = read_desc_file(benchmark_dir)
    min_rules = params["min_rules"]
    max_rules = params["max_rules"]
    runs = params["runs"]
    increment = params["increment"]
    num_traces = params["num_traces"]

    lines = []
    for num_rules in range(min_rules, max_rules + 1, increment):
        for timeout in range(min_timeout, max_timeout + 1, timeout_inc):
            for run in range(runs):
                for trace in range(num_traces):
                    print "num_rules: %d, run: %d, trace: %d, timeout: %d" %\
                            (num_rules, run, trace, timeout)
                    # use unmodified ruleset
                    std_count = do_performance_test(
                            num_rules, run, trace, timeout, False, benchmark_dir, "")
                    print "unmodified           : %d" % std_count
                    # use unmodified worst case
                    std_count_worst = do_performance_test(
                            num_rules, run, trace, timeout, True, benchmark_dir, "")
                    print "unmodified worst case: %d" % std_count_worst
                    # use modified ruleset
                    mod_count = do_performance_test(
                            num_rules, run, trace, timeout, False,
                            benchmark_dir, "_out")
                    print "modified             : %d" % mod_count
                    # use modified ruleset worst case
                    mod_count_worst = do_performance_test(
                            num_rules, run, trace, timeout, True,
                            benchmark_dir, "_out")
                    print "modified worst case  : %d" % mod_count_worst

                    lines.append("%d, %d, %d, %d, %d, %d, %d, %d" % (num_rules,
                            timeout, run, trace, std_count, mod_count,
                            std_count_worst, mod_count_worst))
    clean_remote_firewall_rules()
    result_fn = os.path.join(benchmark_dir, "RESULTS.txt")
    with open(result_fn, "w") as result_f:
        result_f.write("\n".join(lines))


if __name__ == "__main__":
    main(sys.argv)
