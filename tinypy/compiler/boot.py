import sys

STDIN = object()
STDOUT = object()

if not "tinypy" in sys.version:
    import struct
    _bytes = bytes

    ARGV = sys.argv

    def join(v):
        # always return bytes.
        out = b''
        for el in v:
            # At this point all elements in v must be bytes.
            if not isinstance(el, _bytes):
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
        if not isinstance(v, _bytes):
            raise
        if b'.' in v:
            return float(v)
        else:
            if v[0:2] == b'0x':
                return int(v[2:],16)
            return int(v)

    def istype(v,t):
        if t == 'string': return isinstance(v,str)
        elif t == 'list': return (isinstance(v,list) or isinstance(v,tuple))
        elif t == 'dict': return isinstance(v,dict)
        elif t == 'number': return (isinstance(v,float) or isinstance(v,int))
        raise '?'

    def pack(format, v):
        return struct.pack(format, v)

    def unpack(format, v):
        return struct.unpack(format, v)[0]

    def load(fname):
        if fname is STDIN:
            if sys.version_info.major == 2:
                f = sys.stdin
            else:
                f = sys.stdin.buffer
        else:
            f = open(fname,'rb')
        r = f.read().decode()
        f.close()
        return r

    def read(fname):
        if fname is STDIN:
            if sys.version_info.major == 2:
                f = sys.stdin
            else:
                f = sys.stdin.buffer
        else:
            f = open(fname,'rb')
        r = f.read()
        f.close()
        return r

    if sys.version_info.major == 2:
        def bytes(v):
            return _bytes(bytearray(v))

    def save(fname,v):
        if fname is STDOUT:
            if sys.version_info.major == 2:
                f = sys.stdout
            else:
                f = sys.stdout.buffer
        else:
            f = open(fname,'wb')
        f.write(v)
        f.close()

    def string_escape(bytes):
        s = ''
        for b in bytes:
            if b >= 32 and b < 127:
                s = s + chr(b)
            else:
                s = s + '\\x%02X' % b
        return s

else:
    from tinypy.runtime.builtins import *
    from os import load, read, save

    def join(v):
        return ''.join(v)

    def merge(a, b):
        a.update(b)
