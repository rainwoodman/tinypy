from tinypy.compiler.boot import *

import tinypy.compiler.tokenize as tokenize
import tinypy.compiler.parse as parse
import tinypy.compiler.encode as encode

def _compile(s,fname):
    tokens = tokenize.tokenize(s)
    t = parse.parse(s,tokens)
    r = encode.encode(fname,s,t)
    return r

def import_fname(fname,name):
    g = {}
    g['__name__'] = name
    MODULES[name] = g
    s = load(fname)
    code = _compile(s,fname)
    g['__code__'] = code
    exec(code,g)
    return g

def tinypy():
    return import_fname(ARGV[0],'__main__')

def main(src,dest):
    s = read(src)
    r = _compile(s,src)
    save(dest,r)

if __name__ == '__main__':
    main(ARGV[1],ARGV[2])
#    main(ARGV[1],ARGV[2])
#    main(ARGV[1],ARGV[2])
#    main(ARGV[1],ARGV[2])
#    main(ARGV[1],ARGV[2])
#    main(ARGV[1],ARGV[2])
#    main(ARGV[1],ARGV[2])
#    main(ARGV[1],ARGV[2])
