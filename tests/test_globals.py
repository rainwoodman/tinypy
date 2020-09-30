from tinypy.runtime.testing import UnitTest

foo = 3
class MyTest(UnitTest):
    def setup(self, testname):
        print('setup', testname)

    def teardown(self, testname):
        print('teardown', testname)

    def test_globals(self):
        assert 'foo' in globals()
        assert 'bar' not in globals()

t = MyTest()

t.run()
