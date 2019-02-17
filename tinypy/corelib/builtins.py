import sys
import os
from tinypy.language.builtins import *

class Exception:
    def __init__(self, message):
        self.message = message
    def __repr__(self):
        return self.message

class ImportError(Exception):
    pass    

def dict(**kwargs):
    r = {}
    if istype(kwargs, 'dict'):
        r.update(kwargs)
    else:
        for item in kwargs:
            k, v = item
            r[k] = item
    return r

def format(a, b):
    r = []
    i = 0
    j = 0
    while i < len(a):
        if a[i] == '{':
            r.append(a[j:i])
            j = i
            while j < len(a):
                if a[j] == '}':
                    break
                j = j + 1

            name = a[i+1:j]
            r.append(str(b[name]))
            i = j
        i = i + 1
    return ''.join(r) 

def import_fname(fname,name):
    g = {}
    g['__name__'] = name
    sys.modules[name] = g
    s = os.load(fname)
    code = compile(s,fname)
    g['__code__'] = code
    exec(code, g)
    return g

def _entry_point():
    return import_fname(sys.argv[0], '__main__')
            
