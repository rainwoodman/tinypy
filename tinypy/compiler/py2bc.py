from tinypy.compiler.boot import *
import tinypy.compiler.tokenize as tokenize
import tinypy.compiler.parse as parse
import tinypy.compiler.encode as encode

def compile(s, fname):
    tokens = tokenize.tokenize(s)
    t = parse.parse(s,tokens)
    r = encode.encode(fname,s,t)
    return r

def main(src, dest):
    s = read(src)
    r = compile(s,src)
    save(dest, r)
