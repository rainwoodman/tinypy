import sys
from tinypy.runtime.types import Exception

def fgcolor(code):
    ESC = chr(27)
    if code:
        return ESC + "[1;" + str(code) + "m"
    else:
        return ESC + "[0m"

class TestResult:
    def __init__(self, name, status, exc):
        self.status = status
        self.name = name
        self.exc = exc
    def __str__(self):
        return '{name} ........ {status}'.format(getraw(self))

class TestError(Exception):
    pass

class UnitTest:
    def __init__(self):
        pass

    def discover(self, test_prefix):
        r = []
        meta = getmeta(self)
        for name in meta:
            if name.find(test_prefix) == 0:
                r.append(name)
        return r

    def setup(self, testname):
        pass

    def teardown(self, testname):
        pass

    def run(self, monitor=print):
        tests = self.discover("test_")
        tests.sort()

        subst = dict(script=sys.argv[0], total=len(tests))

        subst['RED'] = fgcolor(31)
        subst['GREEN'] = fgcolor(32)
        subst['RESET'] = fgcolor(0)

        monitor("==== Testing {script} Started {total} cases ====".format(subst))

        nfail = 0
        for test in tests:
            testfunc = self[test]
            result = self.runone(test, testfunc)
            if result.status != "OK":
                nfail = nfail + 1
            subst['name'] = test
            subst['status'] = result.status
            msg = "[ {status} ] {name}".format(subst)
            monitor(msg)

        subst['nfail'] = nfail
        if nfail > 0:
            msg = "==== Testing {script} {RED}{nfail} Failed{RESET} ====".format(subst)
        else:
            msg = "==== Testing {script} {GREEN}All Passed{RESET} ====".format(subst)

        monitor(msg)
        if nfail > 0:
            raise TestError(msg)

    def runone(self, test, testfunc):
        self.setup(test)
        try:
            testfunc(self)
            return TestResult(test, "OK", None)
        except:
            return TestResult(test, "FAIL")
        self.teardown(test)
