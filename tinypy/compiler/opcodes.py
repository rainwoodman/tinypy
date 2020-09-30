
EOF,ADD,SUB,MUL,DIV,POW,BITAND,BITOR,CMP,MGET,GET,SET,NUMBER,STRING,GGET,GSET,MOVE,DEF,PASS,JUMP,CALL,RETURN,IF,DEBUG,EQ,LE,LT,DICT,LIST,NONE,LEN,LINE,PARAMS,IGET,FILE,NAME,NE,HAS,RAISE,SETJMP,MOD,LSH,RSH,ITER,DEL,REGS,BITXOR,IFN,NOT,BITNOT = 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48, 49

def _make_dicts():
    names = {}
    codes = {}
    G = globals()
    for k in G:
        if not (k[0] >= 'A' and k[0] <= 'Z'):
            continue
        names[G[k]] = k
        codes[k] = G[k]
    return names, codes

names, codes = _make_dicts()

def create_ccode():
    lines = []
    cases = []

    for name in codes:
        cval = codes[name]
        cname = "TP_I" + name
        lines.append("#define " + cname + " " + str(cval) + " ")
        cases.append("case " + cname + ': return "' + name + '";')

    cases.append("default : return NULL;");
    header = "/* Generated from opcodes.py with tpc -x. Do not modify. */\n"
    enums = '\n'.join(lines)
    translate = ("char * tp_get_opcode_name(int opcode) {\n"
       + "switch(opcode) {\n"
       + '\n'.join(cases)
       + "\n}\n}\n"
    )
    return '\n'.join([header, enums, translate])
