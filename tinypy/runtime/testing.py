class TestResult:
    def __init__(self, name, status):
        self.status = status
        self.name = name
    def __str__(self):
        return self.name + ': ....... [ ' + self.status + ' ]'


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

        for test in tests:
            testfunc = self[test]
            result = self.runone(test, testfunc)

            if monitor is not None:
                monitor(result)

    def runone(self, test, testfunc):
        self.setup(test)
        try:
            testfunc(self)
            return TestResult(test, "OK")
        except:
            return TestResult(test, "FAIL")
        self.teardown(test)
            

