from tinypy.compiler.boot import *

import tinypy.compiler.tokenize as tokenize
import tinypy.compiler.parse as parse
import tinypy.compiler.encode as encode

def _compile(s,fname):
    tokens = tokenize.tokenize(s)
    t = parse.parse(s,tokens)
    r = encode.encode(fname,s,t)
    return r

def main(src,dest):
    s = read(src)
    r = _compile(s,src)
    save(dest,r)

if __name__ == '__main__':
    main(ARGV[1],ARGV[2])
