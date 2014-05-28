import os
import sys
import re
import argparse

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
        for run_id in range(self.runs_):
            for rule_num in rule_nums:
                cb_filename = os.path.join(self.target_dir_,
                        "%d_%d.cb" % (run_id, rule_num))
                cb_cmd = "db_generator -r %d %s > /dev/null" % \
                        (rule_num, cb_filename)
                os.system(cb_cmd)
                self.translate_cb_to_iptables(cb_filename)

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
    rule_generator = RuleGenerator(os.path.abspath(args.destination),
            args.min_rules, args.max_rules, args.increment,
            args.runs)
    rule_generator.generate()
        

if __name__ == "__main__":
    main(sys.argv[1:])
