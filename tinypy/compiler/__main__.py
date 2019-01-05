from tinypy.compiler import py2bc
from tinypy.compiler.boot import *

def main(args=None):
    if args is None: args = ARGV
    if len(args) == 2:
        src = args[1]
        for i in range(len(args[1]) - 1, 0, -1):
            if args[1][i] == '.': break

        dest = args[1][:i] + '.tpc'
    elif len(args) == 3:
        src = args[1]
        dest = args[2]
    else:
        print("Usage : tinypyc source.py [target.tpc]")
        return
    py2bc.main(src, dest)

if __name__ == '__main__':
    main()
