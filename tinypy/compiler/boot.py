import sys

if not "tinypy" in sys.version:

    ARGV = sys.argv

    def join(v):
        # always return bytes.
        out = b''
        for el in v:
            # At this point all elements in v must be bytes.
            if isinstance(el, str):
                raise
                el = el.encode()
            out += el
        return out

    def merge(a,b):
        if isinstance(a, dict):
            for k in b: a[k] = b[k]
        else:
            for k in b: setattr(a,k,b[k])

    def number(v):
        if type(v) is str and v[0:2] == '0x':
            v = int(v[2:],16)
        return float(v)

    def istype(v,t):
        if t == 'string': return isinstance(v,str)
        elif t == 'list': return (isinstance(v,list) or isinstance(v,tuple))
        elif t == 'dict': return isinstance(v,dict)
        elif t == 'number': return (isinstance(v,float) or isinstance(v,int))
        raise '?'

    def fpack(v):
        import struct
        return struct.pack('d', v)

    def funpack(v):
        import struct
        return struct.unpack('d', v)[0]

    def load(fname):
        f = open(fname,'rb')
        r = f.read().decode()
        f.close()
        return r

    def read(fname):
        f = open(fname,'rb')
        r = f.read().decode()
        f.close()
        return r

    def save(fname,v):
        f = open(fname,'w')
        f.write(v)
        f.close()
else:
    from __builtins__ import *
    from os import load, read, save

    def join(v):
        return ''.join(v)

    def merge(a, b):
        a.update(b)

    # tpy strings are byte strings.
    def bytes(s):
        return s

    def bytearray(t):
        return join([chr(i) for i in t])
