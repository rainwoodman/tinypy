import sys
from tinypy.runtime.types import Exception

def fgcolor(code):
    ESC = chr(27)
    if code:
        return ESC + "[1;" + str(code) + "m"
    else:
        return ESC + "[0m"

class TestResult:
    def __init__(self, name, passed, exc):
        self.passed = passed
        self.name = name
        self.exc = exc

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

        monitor("{GREEN}[ STARTED ]{RESET} {script} {total} cases.".format(subst))

        nfail = 0
        itest = 0
        for test in tests:
            testfunc = self[test]
            result = self.runone(test, testfunc)
            subst['name'] = test
            if result.passed:
                subst['status'] = "PASS"
            else:
                nfail = nfail + 1
                subst['status'] = "FAIL"
            subst['id'] = itest
            msg = "[ {status} ] {id}: {name}".format(subst)
            itest = itest + 1
            monitor(msg)

        subst['nfail'] = nfail
        if nfail > 0:
            msg = "{RED}[ FAIL ]{RESET} {script} {nfail} Failed.".format(subst)
        else:
            msg = "{GREEN}[ HEALTHY ]{RESET} {script}.".format(subst)

        monitor(msg)
        if nfail > 0:
            return False
        return True

    def runone(self, test, testfunc):
        self.setup(test)
        try:
            testfunc(self)
            return TestResult(test, True, None)
        except:
            return TestResult(test, False, None)
        self.teardown(test)
