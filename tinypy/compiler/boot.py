import sys

if not "tinypy" in sys.version:

    ARGV = sys.argv

    def join(v):
        out = b''
        for el in v:
            try:
                out += el
            except TypeError:
                out += el.encode('latin1')
        return out

    def merge(a,b):
        if isinstance(a,dict):
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

    def system(cmd):
        import os
        return os.system(cmd)

    def load(fname):
        f = open(fname,'rb')
        r = f.read()
        f.close()
        return r

    def read(fname):
        f = open(fname,'r')
        r = f.read()
        f.close()
        return r

    def save(fname,v):
        f = open(fname,'wb')
        f.write(v)
        f.close()
else:
    def join(v): return BUILTINS.join(v)
    def merge(a,b): return BUILTINS.merge(a, b)
    def number(v): return BUILTINS.number(v)
    def istype(v,t): return BUILTINS.istype(v, t)
    def fpack(v): return BUILTINS.fpack(v)
    def system(cmd): return BUILTINS.system(cmd)
    def load(fname): return BUILTINS.load(fname)
    def read(fname): return BUILTINS.read(fname)
    def save(fname,v): return BUILTINS.save(fname, v)

    class Exception:
        def __init__(self, message):
            self.message = message
        def __repr__(self):
            return self.message

    def funpack(bytes):
        def eat(x, bit):
            y = int(x / 2 ** bit)
            x -= y * 2 ** bit
            return x, y
        x = 0
        for i in range(8):
            x += ord(bytes[i]) * 2 ** (i * 8)
        x, sign = eat(x, 63)
        x, exponent = eat(x, 52)
        x, mantissa1 = eat(x, 31)
        x, mantissa2 = eat(x, 0)
        mantissa = mantissa1 * 2 ** 31 + mantissa2
        sign = sign * -2 + 1
        x = sign * 2 ** (exponent - 1023) * (1 + mantissa / 2 ** 52)
        return x
