import sys
from tinypy.runtime.types import Exception

def fgcolor(code):
    ESC = chr(27)
    if code:
        return ESC + "[1;" + str(code) + "m"
    else:
        return ESC + "[0m"

class TestResult:
    def __init__(self, name, passed, exc, stack):
        self.passed = passed
        self.name = name
        self.exc = exc
        self.stack = stack

class TestError(Exception):
    pass

class UnitTest:
    def __init__(self):
        self._known_failure_prefix = 'known_failure_test'
        self._test_prefix = 'test_'

    def discover(self, test_prefix, known_failure_prefix):
        r = []
        meta = getmeta(self)
        for name in meta:
            if name.find(known_failure_prefix) == 0:
                r.append(name)
            if name.find(test_prefix) == 0:
                r.append(name)
        return r

    def setup(self, testname):
        pass

    def teardown(self, testname):
        pass

    def run(self, monitor=print):
        tests = self.discover(self._test_prefix, self._known_failure_prefix)
        tests.sort()

        subst = dict(script=sys.argv[0], total=len(tests))

        subst['RED'] = fgcolor(31)
        subst['GREEN'] = fgcolor(32)
        subst['RESET'] = fgcolor(0)

        monitor("{GREEN}[ STARTED ]{RESET} {script} {total} cases.".format(subst))

        nfail = 0
        nknownfail = 0
        itest = 0
        for test in tests:
            testfunc = self[test]
            result = self.runone(test, testfunc)
            subst['name'] = test
            if result.passed:
                subst['status'] = "PASS"
            elif test.startswith(self._known_failure_prefix):
                nknownfail = nknownfail + 1
                subst['status'] = "KNOWN"
            else:
                nfail = nfail + 1
                subst['status'] = "FAIL"
            subst['id'] = itest
            msg = "[ {status} ] {id}: {name}".format(subst)
            itest = itest + 1
            monitor(msg)
            if not result.passed:
                monitor("Exception:\n{exc}\n{stack}".format(result))

        subst['nfail'] = nfail
        subst['nknownfail'] = nknownfail
        if nfail > 0:
            msg = "{RED}[ FAIL ]{RESET} {script} {nfail} Failed.".format(subst)
        else:
            msg = "{GREEN}[ HEALTHY ]{RESET} {script} {nknownfail} Known Failures.".format(subst)

        monitor(msg)
        if nfail > 0:
            return False
        return True

    def runone(self, test, testfunc):
        self.setup(test)
        try:
            testfunc(self)
            return TestResult(test, True, None, None)
        except:
            exc, stack = sys.get_exc()
            return TestResult(test, False, exc, stack)
        self.teardown(test)
