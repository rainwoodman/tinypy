# Control flow
EOF = 0x00
MOVE = 0x01
PASS = 0x02
REGS = 0x03
JUMP = 0x04
CALL = 0x05
PARAMS = 0x06
RETURN = 0x07
SETJMP = 0x08
IF = 0x09
IFN = 0x0a
RAISE = 0x0b

# Binary ops
ADD = 0x10
SUB = 0x11
MUL = 0x12
DIV = 0x13
POW = 0x14
MOD = 0x15
LSH = 0x16
RSH = 0x17

BITAND = 0x18
BITOR = 0x19
BITXOR = 0x1a

EQ = 0x20
LE = 0x21
LT = 0x22
NE = 0x23
IN = 0x24
NOTIN = 0x25

# Unary ops
LEN = 0x30
NOT = 0x31
BITNOT = 0x32

# get set
GET = 0x40
MGET = 0x41
GGET = 0x42
IGET = 0x43

SET = 0x44
GSET = 0x45
DEL = 0x46

# operation
UPDATE = 0x47

# iteration
ITER = 0x48

# objects
NONE = 0x50
NUMBER = 0x51
STRING = 0x52
LIST = 0x53
DICT = 0x54
DEF = 0x55
CLASS = 0x56

# debug
ASSERT = 0x80
FILE = 0x81
NAME = 0x82
VAR = 0x83
LINE = 0x84

def _make_dicts():
    names = {}
    codes = {}
    G = globals()
    for k in G:
        if not (k[0] >= 'A' and k[0] <= 'Z'):
            continue
        names[G[k]] = k
        if k in codes:
            raise Exception("duplicated codes")
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
