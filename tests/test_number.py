import sys
from tinypy.runtime.testing import UnitTest

class NumberUnitTests(UnitTest):

    def test_float(self):
        assert float(3.1) == 3.1
        assert float(-3.9) == -3.9
        assert float("3") == 3
        assert float("3.0") == 3.0
        assert float("-3.9") == -3.9

    def test_int(self):
        assert int(3.0) == 3.0
        assert int(-3.0) == -3.0
        assert int("3") == 3
        assert int("3", 10) == 3
        assert int("3.0") == 3.0
        assert int("-3.0") == -3.0

        assert 0x0a == 10
        assert int("3", 16) == 3
        assert int("a", 16) == 10

    def known_failure_test_large_int(self):
        # 64bit integer support needs to add a new int type.
        assert 0xffffffffffffffff == -1
        assert 0xffffffffffffffff == 18446744073709551615
        assert int("ffffffffffffffff") == 18446744073709551615

    def test_badnumber(self):
        try:
            int("a3")
        except:
            exc, stack = sys.get_exc()
            assert "TypeError" in exc

        try:
            float("3ffff")
        except:
            exc, stack = sys.get_exc()
            assert "TypeError" in exc
        assert int("3.0") == 3.0
        assert int("-3.0") == -3.0

    def test_abs(self):
        assert abs(3.0) == 3.0
        assert abs(-3.0) == 3.0
        assert abs("3") == 3
        assert abs("3.0") == 3.0
        assert abs("-3.0") == 3.0

    def test_round(self):
        # these depend on the fpu flags I think.
        assert round(3.0) == 3.0
        assert round(-3.0) == -3.0
        assert round(-3.9) == -4.0
        assert round(3.9) == 4.
        assert round(3.1) == 3.
        assert round(-3.1) == -3.

if __name__ == '__main__':
    tests = NumberUnitTests()
    tests.run()

