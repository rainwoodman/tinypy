import tinypy.compiler.tokenize as tokenize
from tinypy.compiler.tokenize import Token
from tinypy.compiler.boot import *
from tinypy.compiler.opcodes import *

class DState:
    def __init__(self,code,fname):
        self.nopos = False
        self.code, self.fname = code,fname
        self.lines = self.code.split('\n')

        self.stack,self.out,self._scopei,self.tstack,self._tagi,self.data = [],[('tag','EOF')],0,[],0,{}
        self.error = False
    def begin(self,gbl=False):
        if len(self.stack): self.stack.append((self.vars,self.r2n,self.n2r,self._tmpi,self.mreg,self.snum,self._globals,self.lineno,self.globals,self.rglobals,self.cregs,self.tmpc))
        else: self.stack.append(None)
        self.vars,self.r2n,self.n2r,self._tmpi,self.mreg,self.snum,self._globals,self.lineno,self.globals,self.rglobals,self.cregs,self.tmpc = [],{},{},0,0,str(self._scopei),gbl,-1,[],[],['regs'],0
        self._scopei += 1
        insert(self.cregs)
    def end(self, eof=True):
        # cregs is already inserted to out; this modifies it in place.
        self.cregs.append(self.mreg)
        if eof:
            code(EOF)

        if self.tmpc != 0:
            print("Warning:\nencode.py contains a register leak\n")
        # This next line forces the encoder to
        # throw an exception if any tmp regs 
        # were leaked within the frame
        assert(self.tmpc == 0) #REG

        if len(self.stack) > 1:
            self.vars,self.r2n,self.n2r,self._tmpi,self.mreg,self.snum,self._globals,self.lineno,self.globals,self.rglobals,self.cregs,self.tmpc = self.stack.pop()
        else: self.stack.pop()


def insert(v): D.out.append(v)
def write(v):
    if istype(v,'list'):
        insert(v)
        return
    for n in range(0,len(v),4):
        insert(('data',v[n:n+4]))
def setpos(v):
    if D.nopos: return
    line,x = v
    if line == D.lineno: return
    text = D.lines[line-1]
    D.lineno = line
    val = text.encode() + b"\0"*(4-len(text)%4)
    code_16(LINE,int(len(val)/4),line)
    write(val)
def code(i,a=0,b=0,c=0):
    if not istype(i,'number'): raise
    if not istype(a,'number'): raise
    if not istype(b,'number'): raise
    if not istype(c,'number'): raise
    write(('code',i,a,b,c))
def code_16(i,a,b):
    if b < 0: b += 0x8000
    code(i,a,(b&0xff00)>>8,(b&0xff)>>0)
def get_code16(i,a,b):
    return ('code',i,a,(b&0xff00)>>8,(b&0xff)>>0)

def _do_string(v,r=None):
    r = get_tmp(r)
    val = v.encode() + b"\0"*(4-len(v)%4)
    code_16(STRING,r,len(v))
    write(val)
    return r
def do_string(t,r=None):
    return _do_string(t.val,r)

def _do_number(v,r=None):
    r = get_tmp(r)
    code(NUMBER,r,0,0)
    write(fpack(number(v.encode())))
    return r
def do_number(t,r=None):
    return _do_number(t.val,r)

def get_tag():
    k = str(D._tagi)
    D._tagi += 1
    return k
def stack_tag():
    k = get_tag()
    D.tstack.append(k)
    return k
def pop_tag():
    D.tstack.pop()

def tag(*t):
    t = D.snum+':'+':'.join([str(v) for v in t])
    insert(('tag',t))
def jump(*t):
    t = D.snum+':'+':'.join([str(v) for v in t])
    insert(('jump',t))
def setjmp(*t):
    t = D.snum+':'+':'.join([str(v) for v in t])
    insert(('setjmp',t))
def fnc(*t):
    t = D.snum+':'+':'.join([str(v) for v in t])
    r = get_reg(t)
    insert(('fnc',r,t))
    return r

def map_tags():
    tags = {}
    out = []
    n = 0
    for item in D.out:
        if item[0] == 'tag':
            tags[item[1]] = n
            continue
        if item[0] == 'regs':
            out.append(get_code16(REGS,item[1],0))
            n += 1
            continue
        out.append(item)
        n += 1
    for n in range(0,len(out)):
        item = out[n]
        if item[0] == 'jump':
            out[n] = get_code16(JUMP,0,tags[item[1]]-n)
        elif item[0] == 'setjmp':
            out[n] = get_code16(SETJMP,0,tags[item[1]]-n)
        elif item[0] == 'fnc':
            out[n] = get_code16(DEF,item[1],tags[item[2]]-n)
    for n in range(0,len(out)):
        item = out[n]
        if item[0] == 'data':
            out[n] = item[1]
        elif item[0] == 'code':
            i,a,b,c = item[1:]
            out[n] = bytes(bytearray((i,a,b,c)))
        else:
            raise str(('huh?',item))
        if len(out[n]) != 4:
            raise ('code '+str(n)+' is wrong length '+str(len(out[n])))
    D.out = out

def get_tmp(r=None):
    if r != None: return r
    return get_tmps(1)[0]
def get_tmps(t):
    rs = alloc(t)
    regs = range(rs,rs+t)
    for r in regs:
        set_reg(r,"$"+str(D._tmpi))
        D._tmpi += 1
    D.tmpc += t #REG
    return regs
def alloc(t):
    s = ''.join(["01"[r in D.r2n] for r in range(0,min(256,D.mreg+t))])
    return s.index('0'*t)
def is_tmp(r):
    if r is None: return False
    return (D.r2n[r][0] == '$')
def un_tmp(r):
    n = D.r2n[r]
    free_reg(r)
    set_reg(r,'*'+n)
def free_tmp(r):
    if is_tmp(r): free_reg(r)
    return r
def free_tmps(r):
    for k in r: free_tmp(k)
def get_reg(n):
    if n not in D.n2r:
        set_reg(alloc(1),n)
    return D.n2r[n]
def set_reg(r,n):
    D.n2r[n] = r; D.r2n[r] = n
    D.mreg = max(D.mreg,r+1)
    if not is_tmp(r):
      val = n.encode() + b"\0"*(4-len(n)%4)
      code_16(VAR, r, len(n))
      write(val)
    return r

def free_reg(r):
    if is_tmp(r): D.tmpc -= 1
    n = D.r2n[r]; del D.r2n[r]; del D.n2r[n]

def imanage(orig,fnc):
    items = orig.items
    orig.val = orig.val[:-1]
    t = Token(orig.pos,'symbol','=',[items[0],orig])
    return fnc(t)

def unary(i,tb,r=None):
    r = get_tmp(r)
    b = do(tb)
    code(i,r,b)
    if r != b: free_tmp(b)
    return r

def infix(i,tb,tc,r=None):
    r = get_tmp(r)
    b,c = do(tb,r),do(tc)
    code(i,r,b,c)

    if r != b: free_tmp(b)
    free_tmp(c)
    return r

def infix_traced(i,tb,tc,r=None):
    r = get_tmp(r)
    b,c = do(tb),do(tc)
    code(i,r,b,c)

    return r, b, c

def logic_infix(op, tb, tc, _r=None):
    t = get_tag() 
    r = do(tb, _r)
    if _r != r: free_tmp(_r) #REG
    if op == 'and':   code(IF, r)
    elif op == 'or':  code(IFN, r)
    jump(t, 'end')
    _r = r
    r = do(tc, _r)
    if _r != r: free_tmp(_r) #REG
    tag(t, 'end')
    return r

def _do_none(r=None):
    r = get_tmp(r)
    code(NONE,r)
    return r

def do_cmp(t, trace, r=None):
    items = t.items
    b,c = items[0],items[1]
    v = t.val
    if v[0] in ('>','>='):
        b,c,v = c,b,'<'+v[1:]
    cd = EQ
    if v == '<': cd = LT
    if v == '<=': cd = LE
    if v == '!=': cd = NE
    if v == 'is': cd = EQ
    if v == 'isnot': cd = NE
    if v == 'in':
        cd = IN
    if v == 'notin':
        cd = NOTIN
    if trace:
        return infix_traced(cd,b,c,r)
    else:
        return infix(cd,b,c,r)

def do_symbol(t,r=None):
    sets = ['=']
    isets = ['+=','-=','*=','/=', '|=', '&=', '^=']
    cmps = ['<','>','<=','>=','==','!=', 'in', 'is', 'isnot', 'notin']
    metas = {
        '+':ADD,'*':MUL,'/':DIV,'**':POW,
        '-':SUB,
        '%':MOD,'>>':RSH,'<<':LSH,
        '&':BITAND,'|':BITOR,'^':BITXOR,
    }
    if t.val == 'None': return _do_none(r)
    if t.val == 'True':
        return _do_number('1',r)
    if t.val == 'False':
        return _do_number('0',r)
    items = t.items

    if t.val in ['and','or']:
        return logic_infix(t.val, items[0], items[1], r)
    if t.val in isets:
        return imanage(t,do_symbol)
    if t.val == 'not':
        return unary(NOT, items[0], r)
    if t.val in sets:
        return do_set_ctx(items[0],items[1]);
    elif t.val in cmps:
        return do_cmp(t, False, r)
    else:
        return infix(metas[t.val],items[0],items[1],r)

def do_set_ctx(k,v):
    if k.type == 'name':
        if (D._globals and k.val not in D.vars) or (k.val in D.globals):
            c = do_string(k)
            b = do(v)
            code(GSET,c,b)
            free_tmp(c)
            free_tmp(b)
            return
        a = do_local(k)
        b = do(v)
        code(MOVE,a,b)
        free_tmp(b)
        return a
    elif k.type in ('tuple','list'):
        if v.type in ('tuple','list'):
            n,tmps = 0,[]
            for kk in k.items:
                vv = v.items[n]
                tmp = get_tmp(); tmps.append(tmp)
                r = do(vv)
                code(MOVE,tmp,r)
                free_tmp(r) #REG
                n+=1
            n = 0
            for kk in k.items:
                vv = v.items[n]
                tmp = tmps[n]
                free_tmp(do_set_ctx(kk,Token(vv.pos,'reg',tmp))) #REG
                n += 1
            return

        r = do(v); un_tmp(r)
        n, tmp = 0, Token(v.pos,'reg',r)
        for tt in k.items:
            free_tmp(do_set_ctx(tt,Token(tmp.pos,'get',None,[tmp,Token(tmp.pos,'number',str(n))]))) #REG
            n += 1
        free_reg(r)
        return

    r = do(k.items[0])
    rr = do(v)
    tmp = do(k.items[1])
    code(SET,r,tmp,rr)
    free_tmp(r) #REG
    free_tmp(tmp) #REG
    return rr

def manage_seq(i,a,items,sav=0):
    l = max(sav,len(items))
    n,tmps = 0,get_tmps(l)
    for tt in items:
        r = tmps[n]
        b = do(tt,r)
        if r != b:
            code(MOVE,r,b)
            free_tmp(b)
        n +=1
    if not len(tmps):
        code(i,a,0,0)
        return 0
    code(i,a,tmps[0],len(items))
    free_tmps(tmps[sav:])
    return tmps[0]

def p_filter(items):
    """ split items to p, n, l, d.

    for example: (1, a=1, *args, **kwargs)
        p: [1]
        n: [a=1]
        l: args
        d: kwargs
    """
    p,n,l,d = [],[],None,None
    for t in items:
        if t.type == 'symbol' and t.val == '=': n.append(t)
        elif t.type == 'largs': l = t
        elif t.type == 'dargs': d = t
        else: p.append(t)
    return p,n,l,d

def do_import(t):
    if len(t.items) == 1:
        mod = t.items[0]
        name = mod
    else:
        mod, name = t.items

    mod.type = 'string'
    v = do_call(Token(t.pos,'call',None,[
        Token(t.pos,'name','__import__'),
        mod,
        Token(t.pos, 'symbol', 'None')]
        ))

    name.type = 'name'
    do_set_ctx(name,Token(t.pos,'reg',v))

def do_from(t):
    mod = t.items[0]
    items = t.items[1]
    mod.type = 'string'

    if items.type == 'args':
        # it really shouldn't be args -- need to fix the parser.
        if items.val == '*':
            items.type = 'string'
    elif items.type == 'name':
        items.type = 'string'
        items = Token(items.pos, 'tuple', None, [items])
    elif items.type ==  'tuple':
        for item in items.items:
            item.type = 'string'
    else: 
        tokenize.u_error('SyntaxError', D.code, t.pos)

    v = do(Token(t.pos,'call',None,[
        Token(t.pos,'name','__import__'),
        mod,
        items,
        ]))

    g = do(Token(t.pos, 'name', '__dict__'))
    code(UPDATE, g, v);
    free_tmp(g)
    free_tmp(v)

def do_globals(t):
    for t in t.items:
        if t.val not in D.globals:
            D.globals.append(t.val)
def do_del(tt):
    for t in tt.items:
        r = do(t.items[0])
        r2 = do(t.items[1])
        code(DEL,r,r2)
        free_tmp(r); free_tmp(r2) #REG

def do_call(t,r=None):
    r = get_tmp(r)
    items = t.items
    fnc = do(items[0])
    p,n,l,d = p_filter(t.items[1:])
    e = None
    if len(n) != 0 or d != None:
        e = do(Token(t.pos,'dict',None,[])); un_tmp(e);
        for i in n:
            i.items[0].type = 'string'
            t1,t2 = do(i.items[0]),do(i.items[1])
            code(SET,e,t1,t2)
            free_tmp(t1); free_tmp(t2) #REG
        if d: 
            f = do(d.items[0])
            un_tmp(f)
            code(MERGE,e,f)
    # FIXME: for CPython compat we shall change the convention
    # to always set the two last PARAMS to * and **,
    # then on the caller side unpack the args.
    manage_seq(PARAMS,r,p)
    if l != None:
        t2 = do(l.items[0])
        code(ADD,r,r,t2)
        free_tmp(t1); free_tmp(t2) #REG
    if e != None:
        t1 = _do_none()
        code(SET,r,t1,e)
        free_tmp(t1) #REG
    code(CALL,r,fnc,r)
    free_tmp(fnc) #REG
    return r

def do_name(t,r=None):
    if t.val in D.vars:
        return do_local(t,r)
    if t.val not in D.rglobals:
        D.rglobals.append(t.val)
    r = get_tmp(r)
    c = do_string(t)
    code(GGET,r,c)
    free_tmp(c)
    return r

def do_local(t,r=None):
    if t.val in D.rglobals:
        D.error = True
        tokenize.u_error('UnboundLocalError',D.code,t.pos)
    if t.val not in D.vars:
        D.vars.append(t.val)
    return get_reg(t.val)

def do_def(tok):
    items = tok.items

    t = get_tag()
    rf = fnc(t,'end')

    D.begin()
    setpos(tok.pos)
    r = do_local(Token(tok.pos,'name','__params__'))  # assigns regs[0] to __params__.
    do_info(items[0].val)
    p,n,l,d = p_filter(items[1].items)
    for i in p:
        v = do_local(i)
        tmp = _do_none()
        code(GET,v,r,tmp)
        free_tmp(tmp) #REG
    for i in n:
        v = do_local(i.items[0])
        do(i.items[1],v)
        tmp = _do_none()
        code(IGET,v,r,tmp)
        free_tmp(tmp) #REG
    if l != None:
        v = do_local(l.items[0])
        tmp = _do_string('*')
        code(GET,v,r,tmp)
        free_tmp(tmp) #REG
    if d != None:
        e = do_local(d.items[0])
        code(DICT,e,0,0)
        tmp = _do_none()
        code(IGET,e,r,tmp)
        free_tmp(tmp) #REG
    free_tmp(do(items[2])) #REG
    D.end()

    tag(t,'end')

    if D._globals: do_globals(Token(tok.pos,0,0,[items[0]]))
    r = do_set_ctx(items[0],Token(tok.pos,'reg',rf))

    free_tmp(rf)

def do_class(tok):
    items = tok.items
    parent = None
    if items[0].type == 'name':
        name = items[0].val
        parent = Token(tok.pos,'name','object')
    else:
        name = items[0].items[0].val
        parent = items[0].items[1]

    kls = get_tmp()
    code(CLASS, kls)
    un_tmp(kls)
    ts = _do_string(name)
    code(GSET,ts,kls)
    free_tmp(ts) #REG
    
    free_tmp(do(Token(tok.pos,'call',None,[
        Token(tok.pos,'name','setmeta'),
        Token(tok.pos,'reg',kls),
        parent])))
    free_reg(kls) #REG

    # define a function for the class body, and call it.
    t = get_tag()
    rf = fnc(t,'end')

    D.begin()
    setpos(tok.pos)
    # Run the class body.
    free_tmp(do(items[1])) #REG
    # merge the local variables to the class:

    # we must refetch the kls object, because after begin() the kls reg is
    # invalidated.
    ts = _do_string(name)
    kls = get_tmp()
    code(GGET, kls, ts)
    un_tmp(kls)
    free_tmp(ts) #REG
    for val in D.vars:
        ts = _do_string(val)
        code(SET,kls, ts, get_reg(val))
        free_tmp(ts) #REG
    free_reg(kls)
    D.end()

    tag(t,'end')

    tmp = _do_none()
    code(CALL, tmp, rf, tmp)
    free_tmp(rf)
    free_tmp(tmp)

def do_while(t):
    items = t.items
    t = stack_tag()
    tag(t,'begin')
    tag(t,'continue')
    r = do(items[0])
    code(IF,r)
    free_tmp(r) #REG
    jump(t,'end')
    free_tmp(do(items[1])) #REG
    jump(t,'begin')
    tag(t,'break')
    tag(t,'end')
    pop_tag()

def do_for(tok):
    items = tok.items

    reg = get_tmp()
    itr = do(items[1])
    i = _do_number('0')

    t = stack_tag(); tag(t,'loop'); tag(t,'continue')
    code(ITER,reg,itr,i); jump(t,'end')
    free_tmp(do_set_ctx(items[0], Token(tok.pos, 'reg', reg)))
    free_tmp(do(items[2])) #REG
    jump(t,'loop')
    tag(t,'break'); tag(t,'end'); pop_tag()

    free_tmp(itr) #REG
    free_tmp(i)

def do_comp(t,r=None):
    name = 'comp:'+get_tag()
    r = do_local(Token(t.pos,'name',name))
    code(LIST,r,0,0)
    key = Token(t.pos,'get',None,[
            Token(t.pos,'reg',r),
            Token(t.pos,'symbol','None')])
    ap = Token(t.pos,'symbol','=',[key,t.items[0]])
    do(Token(t.pos,'for',None,[t.items[1],t.items[2],ap]))
    return r

def do_if(t):
    items = t.items
    t = get_tag()
    n = 0
    for tt in items:
        tag(t,n)
        if tt.type == 'elif':
            a = do(tt.items[0]); code(IF,a); free_tmp(a);
            jump(t,n+1)
            free_tmp(do(tt.items[1])) #REG
        elif tt.type == 'else':
            free_tmp(do(tt.items[0])) #REG
        else:
            raise
        jump(t,'end')
        n += 1
    tag(t,n)
    tag(t,'end')

def do_try(t):
    items = t.items
    t = get_tag()
    setjmp(t,'except')
    free_tmp(do(items[0])) #REG
    code(SETJMP,0)
    jump(t,'end')
    tag(t,'except')
    free_tmp(do(items[1].items[1])) #REG
    tag(t,'end')

def do_return(t):
    if t.items: r = do(t.items[0])
    else: r = _do_none()
    code(RETURN,r)
    free_tmp(r)
    return

def do_assert(t):
    cmps = ['<','>','<=','>=','==','!=', 'in', 'is', 'isnot', 'notin']
    if t.items:
        e = t.items[0]
        if e.type == 'symbol' and e.val in cmps:
            r, b, c = do_cmp(e, True)
        else:
            r = do(e)
            b = _do_none()
            c = _do_none()
    else:
        r = _do_none()
        b = _do_none()
        c = _do_none()
    code(ASSERT,r,b,c);
    free_tmp(r)
    free_tmp(b)
    free_tmp(c)
    return

def do_raise(t):
    if t.items: r = do(t.items[0])
    else: r = _do_none()
    code(RAISE,r)
    free_tmp(r)
    return

def do_statements(t):
    for tt in t.items: free_tmp(do(tt))

def do_list(t,r=None):
    r = get_tmp(r)
    manage_seq(LIST,r,t.items)
    return r

def do_dict(t,r=None):
    r = get_tmp(r)
    manage_seq(DICT,r,t.items)
    return r

def do_get(t,r=None):
    items = t.items
    return infix(GET,items[0],items[1],r)

def do_mget(t,r=None):
    items = t.items
    return infix(MGET,items[0],items[1],r)

def do_break(t): jump(D.tstack[-1],'break')
def do_continue(t): jump(D.tstack[-1],'continue')
def do_pass(t): code(PASS)

def do_info(name='?'):
    if D.nopos: return
    code(FILE,free_tmp(_do_string(D.fname)))
    code(NAME,free_tmp(_do_string(name)))
def do_module(t):
    do_info()
    free_tmp(do(t.items[0])) #REG
def do_reg(t,r=None): return t.val

fmap = {
    'module':do_module,'statements':do_statements,'def':do_def,
    'return':do_return, 'assert':do_assert, 'while':do_while,'if':do_if,
    'break':do_break,'pass':do_pass,'continue':do_continue,'for':do_for,
    'class':do_class,'raise':do_raise,'try':do_try,'import':do_import,
    'globals':do_globals,'del':do_del,'from':do_from,
}
rmap = {
    'list':do_list, 'tuple':do_list, 'dict':do_dict, 'slice':do_list,
    'comp':do_comp, 'name':do_name,'symbol':do_symbol,'number':do_number,
    'string':do_string,'get':do_get, 'mget':do_mget, 'call':do_call, 'reg':do_reg,
}

def do(t,r=None):
    if t.pos: setpos(t.pos)
    #try:
    if 1:
        if t.type in rmap:
            return rmap[t.type](t,r)
        #if r != None: free_reg(r) #REG
        return fmap[t.type](t)
    #except:
    #    raise
    #    if D.error: raise
    #    D.error = True
    #    tokenize.u_error('encode',D.code,t.pos)

def encode(fname,s,t):
    t = Token((1,1),'module','module',[t])
    global D
    s = tokenize.clean(s)
    D = DState(s,fname)
    D.begin(True)
    do(t)
    D.end()
    map_tags()
    out = D.out; D = None
    # Use a function instead of ''.join() so that bytes and
    # strings during bootstrap
    return join(out)

