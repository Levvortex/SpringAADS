#!/usr/bin/env python3
import sys
import os
import time
import argparse

class CustomHelpParser(argparse.ArgumentParser):
    def print_help(self):
        print("""Usage: solution.py [OPTION]...
Delete all files with a specified suffix that have not been accessed for a specified number of days.

Mandatory arguments to long options are mandatory for short options too.
  -s, --suffix SUFFIX        suffix of the files to be deleted
  -d, --days DAYS            number of days since last access
  -p, --path PATH            directory to search in (default: .)
  -h, --help                 display this help and exit""")

    def error(self, message):
        self.print_help()
        sys.exit(0)

def main():
    parser = CustomHelpParser(add_help=False)
    parser.add_argument('-h', '--help', action='store_true')
    parser.add_argument('-s', '--suffix', required=False)
    parser.add_argument('-d', '--days', type=int, required=False)
    parser.add_argument('-p', '--path', default='.')

    args, unknown = parser.parse_known_args()

    if args.help or not args.suffix or args.days is None or unknown:
        parser.print_help()
        sys.exit(0)

    now = time.time()
    days_in_seconds = args.days * 86400

    for root, dirs, files in os.walk(args.path):
        for file in files:
            if file.endswith(args.suffix):
                filepath = os.path.join(root, file)
                try:
                    atime = os.path.getatime(filepath)
                    if (now - atime) >= days_in_seconds:
                        os.remove(filepath)
                except OSError:
                    pass

if __name__ == '__main__':
    main()