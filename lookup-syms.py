import subprocess
import sys

if __name__ == '__main__':
    ego, exe = sys.argv
    f = subprocess.Popen(['addr2line', '-f', '-e', exe],
            stdout = subprocess.PIPE,
            stdin = subprocess.PIPE)
    
    for x in sys.stdin:
        if not x.startswith('MR: malloc('):
            continue
        x = x[4:]
        call, counter, stack = x.strip().split(' : ')
        stack = stack.split(',')
        stack_names = []
        for s in stack:
            f.stdin.write(s + '\n')
            fnname, fnloc = f.stdout.readline().strip(), f.stdout.readline().strip()
            stack_names.append('%s @ %s' % (fnname, fnloc))
        print call, counter, ' -> '.join(stack_names)
