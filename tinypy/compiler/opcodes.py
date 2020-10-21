EOF = 0

ADD = 1
SUB = 2
MUL = 3
DIV = 4
POW = 5
MOD = 40
LSH = 41
RSH = 42

BITAND = 6
BITOR = 7
BITXOR = 46
NOT = 48
BITNOT = 49

MOVE = 16
MGET = 9
GET = 10
SET = 11
ITER = 43

LEN = 30
GGET = 14
GSET = 15
IGET = 33
DEL = 44
UPDATE = 50

DEF = 17
PASS = 18

JUMP = 19
CALL = 20
RETURN = 21
IF = 22
IFN = 47
RAISE = 38
SETJMP = 39

ASSERT = 23

NOTIN = 8
EQ = 24
LE = 25
LT = 26
NE = 36
IN = 37

PARAMS = 32
NUMBER = 12
STRING = 13
DICT = 27
LIST = 28
NONE = 29
CLASS = 52

LINE = 31
FILE = 34
NAME = 35

REGS = 45
VAR = 51

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
