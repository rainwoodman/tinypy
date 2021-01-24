from tinypy.compiler import py2bc
from tinypy.compiler.boot import *
from tinypy.compiler import disasm
from tinypy.compiler import opcodes

def do_shorts(opts, optstring, shortopts, args):
    while optstring != '':
        opt, optstring = optstring[0], optstring[1:]
        if short_has_arg(opt, shortopts):
            if optstring == '':
                if not args:
                    raise Exception('option -%s requires argument' % opt)
                optstring, args = args[0], args[1:]
            optarg, optstring = optstring, ''
        else:
            optarg = ''
        opts.append(('-' + opt, optarg))
    return opts, args

def short_has_arg(opt, shortopts):
    for i in range(len(shortopts)):
        if opt == shortopts[i] != ':':
            return shortopts.startswith(':', i+1)

    raise Exception('option -%s not recognized' % opt)

def getopt(args, shortopts):
    opts = []
    while args and args[0].startswith('-') and args[0] != '-':
        if args[0] == '--':
            args = args[1:]
            break
        opts, args = do_shorts(opts, args[0][1:], shortopts, args[1:])

    return opts, args

def basename(s, stripdir=True):
    if stripdir:
        for j in range(len(s) - 1, -1, -1):
            if j == -1: break
            if s[j] == '/': break
    else:
        j = -1
    for i in range(len(s) - 1, 0, -1):
        if s[i] == '.': break
    return s[j+1:i]

def main(args=None):
    if args is None: args = ARGV
    posargs = []
    options = {} 

    opts, args = getopt(args[1:], 'i:f:n:o:dx')
    opts = dict(opts)
    if len(args) == 1:
        src = args[0]
        out = do_compile(src, opts)
    elif '-x' in opts and len(args) == 0:
        out = do_opcodes(opts)
    else:
        print('Usage tinypyc [-i srcformat] [-f outformat] [-n variable] [-o output_file_name] src.py')
        return 

    if '-o' in opts:
        dest = opts['-o']
    else:
        if '-c' in opts:
            dest = basename(src, False) + '.c'
        else:
            dest = basename(src, False) + '.tpc'

    if dest == '-':
        print(out)
    else:
        save(dest, out)

def do_opcodes(opts):
    return opcodes.create_ccode().encode()

def do_compile(src, opts):
    s = read(src)
    ifmt = opts.get('-i', 'python')
    ofmt = opts.get('-f', 'bytecode')
    if ifmt == 'python':
        data = py2bc.compile(s, src)
    elif ifmt == 'bytecode':
        data = s
    elif ifmt == 'tpasm':
        raise
    else:
        raise "input format must be python, bytecode, or tpasm"

    if ofmt == 'tpasm':
        out = disasm.disassemble(data).encode()
    elif ofmt == 'ccode':
        out = []
        cols = 16
        name = opts.get('-n', '_tp_' + basename(src) + '_tpc').encode()
        out.append(b"""unsigned char %s[] = {""" % name)
        for n in range(0, len(data), cols):
            # FIXME: Py2 and Py3 differs in bytes[i].
            # Py2 returns bytes object. b'c', Py3 returns an int (like C)
            # we need to properly define what tpy shall do as
            # we say in tpy all strings are bytes.
            words = [b"0x%02x" % ord(data[i:i+1])
                for i in range(n, min(n + cols, len(data)))]
            out.append(b",".join(words) + b",")

        out.append(b"""};""")
        out = b'\n'.join(out)
    elif ofmt == 'bytecode':
        out = data
    else:
        raise "output format must be tpasm, ccode, or bytecode"

    return out

if __name__ == '__main__':
    main()
