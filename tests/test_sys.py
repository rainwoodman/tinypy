import sys
from tinypy.runtime.testing import UnitTest

class MyTest(UnitTest):
    def test_no_exc(self):
        assert sys.get_exc()[0] is None

    def test_str_exc(self):
        try:
            raise "Hello"
        except:
            exc, stack = sys.get_exc()
            assert exc == "Hello"
            assert "test_str_exc" in stack

    def test_sys_argv(self):
        assert len(sys.argv) > 0

    def test_sys_conf(self):
        assert sys.conf.gcmax >= 0
        old = sys.conf.gcmax
        sys.conf.gcmax = 16383
        assert sys.conf.gcmax == 16383
        sys.conf.gcmax = old

t = MyTest()

t.run()
