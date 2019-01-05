from tinypy.language.builtins import *
import sys

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
        merge(r, kwargs)
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
    return join(r) 

def compile(s, fname):
    # must be here because at import time of builtin,
    # py2bc is not imported yet
    from tinypy.compiler.py2bc import _compile
    return _compile(s, fname)

def __import__(name):
    # this function overwrites the temporary
    # C based __import__ which only bootstraps the compiler

    if name in sys.modules:
        return sys.modules[name]
    py = name+".py"
    tpc = name+".tpc"
    if exists(py):
        if not exists(tpc) or mtime(py) > mtime(tpc):
            s = load(py)
            code = compile(s,py)
            save(tpc,code)

    if not exists(tpc):
        raise ImportError("Cannot import the compiled script from " +  tpc)

    code = load(tpc)
    g = {'__name__':name,'__code__':code}
    g['__dict__'] = g
    sys.modules[name] = g
    exec(code,g)
    return g

def import_fname(fname,name):
    g = {}
    g['__name__'] = name
    sys.modules[name] = g
    s = load(fname)
    code = compile(s,fname)
    g['__code__'] = code
    exec(code, g)
    return g

def _entry_point():
    return import_fname(sys.argv[0], '__main__')
            
