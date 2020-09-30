from tinypy.runtime.testing import UnitTest

class MyTest(UnitTest):
    def setup(self, testname):
        print('setup', testname)

    def teardown(self, testname):
        print('teardown', testname)

    def test_0_ok(self):
        return

    #def test_1_fail(self):
    #    raise

t = MyTest()

t.run()

