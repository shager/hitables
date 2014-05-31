import os
import sys
import re
import argparse

PI_IP = "141.20.33.253"

def parse_args(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("-r", "--runs", type=int)
    parser.add_argument("-min", "--min-rules", type=int)
    parser.add_argument("-max", "--max-rules", type=int)
    parser.add_argument("-d", "--destination", type=str)
    parser.add_argument("-i", "--increment", type=int)
    args = parser.parse_args(argv)
    if not (args.runs and args.min_rules and args.max_rules and
                args.destination and args.increment):
        parser.print_help()
        sys.exit(1)
    return args


class RuleGenerator(object):

    def __init__(self, target_dir, min_rules, max_rules, increment, runs):
        assert os.path.isdir(target_dir)
        assert os.path.isabs(target_dir)
        self.target_dir_ = target_dir
        self.min_rules_ = min_rules
        self.max_rules_ = max_rules
        self.increment_ = increment
        self.runs_ = runs

    def generate(self):
        rule_nums = range(self.min_rules_, self.max_rules_ + 1,
                self.increment_)
        trace_cmd_template = "trace_generator 1 0.%d 1000 %s"
        trace_cmd_template += " > /dev/null"
        for run_id in range(self.runs_):
            for rule_num in rule_nums:
                cb_filename = os.path.join(self.target_dir_,
                        "%d_%d.cb" % (run_id, rule_num))
                cb_cmd = "db_generator -r %d 1.0 1.0 %s > /dev/null" % \
                        (rule_num, cb_filename)
                cm_cmd = "python gen_rules.py %d %d %d > %s" % (
                        rule_num, 24, run_id, cb_filename)
                os.system(cb_cmd)
                self.adjust_cb_ruleset(cb_filename)
                self.translate_cb_to_iptables(cb_filename)
                for i in range(5):
                    trace_cmd = trace_cmd_template % (i, cb_filename)
                    os.system(trace_cmd)
                    # read number of samples in trace file
                    old_trace_fn = "%s_trace" % cb_filename
                    with open(old_trace_fn, "r") as trace_file:
                        num_packets = len(trace_file.readlines())
                    new_trace_fn = "%s_%d_%d.trace" % (
                            os.path.basename(cb_filename).split(".")[0], i,
                            num_packets)
                    new_trace_fn = os.path.join(self.target_dir_, new_trace_fn)
                    os.system("mv %s %s" % (old_trace_fn, new_trace_fn))

    def adjust_cb_ruleset(self, cb_filename):
        with open(cb_filename, "r") as cb_file:
            lines = cb_file.readlines()
        for i in range(len(lines)):
            lines[i] = self.adjust_cb_line(lines[i])
        with open(cb_filename, "w") as cb_file:
            cb_file.write("".join(lines))
            
    def adjust_cb_line(self, line):
        parts = re.split("[ |\t]", line)
        parts[1] = "%s/32" % PI_IP
        del parts[-1]
        del parts[-1]
        parts[-1] = "0x11/0xFF\n"
        return " ".join(parts)

    def translate_cb_to_iptables(self, cb_path):
        name = os.path.basename(cb_path).split(".")[0]
        dir_ = os.path.dirname(cb_path)
        ipt_name = os.path.join(dir_, "%s.iptables" % name)
        with open(cb_path, "r") as rule_file:
            lines = rule_file.readlines()
        out_lines = []
        with open(ipt_name, "w") as ipt_file:
            out_lines.append("*filter")
            out_lines.append(":INPUT ACCEPT [0:0]")
            out_lines.append(":FORWARD ACCEPT [0:0]")
            out_lines.append(":OUTPUT ACCEPT [0:0]")
            for line in lines:
                line = line[1:]
                parts = re.split("[ |\t]", line)
                out_line = ["-A", "INPUT"]
                out_line.append("--src")
                out_line.append(parts[0])
                out_line.append("--dst")
                out_line.append(parts[1])
                out_line.append("-p")
                out_line.append("udp")
                out_line.append("--sport")
                out_line.append("%s:%s" % (parts[2], parts[4]))
                out_line.append("--dport")
                out_line.append("%s:%s" % (parts[5], parts[7]))
                out_line.append("-j")
                out_line.append("ACCEPT")
                out_line = " ".join(out_line)
                out_lines.append(out_line)
            out_lines.append("COMMIT\n")
            output = "\n".join(out_lines)
            ipt_file.write(output)


def main(argv):
    args = parse_args(argv)
    path = os.path.abspath(args.destination)
    if not os.path.exists(path) or not os.path.isdir(path):
        os.mkdir(path)
    rule_generator = RuleGenerator(path, args.min_rules,
            args.max_rules, args.increment, args.runs)
    rule_generator.generate()
        

if __name__ == "__main__":
    main(sys.argv[1:])
