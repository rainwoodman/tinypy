from tinypy.language.builtins import *

def compile(s, fname):
    from tinypy.compiler.py2bc import _compile
    return _compile(s, fname)

#        join, merge, number, istype, 
#        fpack, system, load, read, save, __import__, setmeta, getmeta)

class Exception:
    def __init__(self, message):
        self.message = message
    def __repr__(self):
        return self.message

oldimport = __import__
def __import__(name):
    if name in MODULES:
        return MODULES[name]
    py = name+".py"
    tpc = name+".tpc"
    if exists(py):
        if not exists(tpc) or mtime(py) > mtime(tpc):
            s = load(py)
            code = _compile(s,py)
            save(tpc,code)
    if not exists(tpc): raise
    code = load(tpc)
    g = {'__name__':name,'__code__':code}
    g['__dict__'] = g
    MODULES[name] = g
    exec(code,g)
    return g
    
    
