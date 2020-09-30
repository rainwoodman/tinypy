import sys
from tinypy.runtime.types import Exception

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

        monitor("=== Testing {script} Started====".format(dict(script=sys.argv[0])))

        nfail = 0
        for test in tests:
            testfunc = self[test]
            result = self.runone(test, testfunc)
            if result.status != "OK":
                nfail = nfail + 1
            if monitor is not None:
                monitor(result)

        msg = "=== Testing {script} {nfail} / {total} Failed ====".format(
                dict(script=sys.argv[0], nfail=nfail, total=len(tests)))
        monitor(msg)
        if nfail > 0:
            raise TestError(msg)

    def runone(self, test, testfunc):
        self.setup(test)
        try:
            testfunc(self)
            return TestResult(test, "OK", None)
        except e:
            print(e)
            return TestResult(test, "FAIL", e)
        self.teardown(test)
