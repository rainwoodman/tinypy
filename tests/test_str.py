import sys
from tinypy.runtime.testing import UnitTest

class StringUnitTests(UnitTest):

    def test_eq(self):
        assert "a" == "a"
        assert "abcdef" == "abcdef"
        assert "abcdef\0" != "abcdef"

    def test_replace(self):
        assert "".replace("a", "b") == ""
        assert "c".replace("a", "b") == "c"
        assert "a".replace("a", "b") == "b"
        assert "aa".replace("a", "b") == "bb"
        assert "ab".replace("ab", "b") == "b"
        assert "aab".replace("ab", "b") == "ab"
        assert "3ab".replace("ab", "b") == "3b"
        assert "3ab1".replace("ab", "b") == "3b1"

    def test_startswith(self):
        a = "012345"
        assert a.startswith("0")
        assert a.startswith("01")
        assert not a.startswith("1")

    def test_format(self):
        a = "{foo}{bar}d".format(dict(foo='abc', bar='123'))
        assert a == "abc123d"

    def test_percent(self):
        a = "{foo}d" % dict(foo='abc')
        assert a == "abcd"

    def test_find(self):
        a = "012345"
        assert a.find("0") == 0
        assert a.find("1") == 1
        assert a.find("9") == -1

    def test_index(self):
        a = "012345"
        assert a.index("0") == 0
        try:
            assert a.index("9") == -1
        except:
            assert "ValueError" in sys.get_exc()[0]

    def test_split(self):
        a = "012345"
        s = a.split('3')
        assert len(s) == 2
        assert s[0] == '012'
        assert s[1] == '45'

    def test_join(self):
        j = ' '.join(['abc', 'def'])
        assert j == 'abc def'

    def test_slice(self):
        assert '0123'[1, 2] == '1'
        assert '0123'[1, None] == '123'
        assert '0123'[None, None] == '0123'
        assert '0123'[None, 1] == '0'
        assert '0123'[None, 2] == '01'

    def test_chr(self):
        j = chr(0) + chr(65) + chr(66) + chr(0)
        assert j == '\0AB\0'
        assert len(j) == 4

    def test_escape(self):
        assert ord("\\") == 92
        assert ord("\n") == 10
        assert ord("\r") == 13
        assert ord("\t") == 9
        assert ord("\0") == 0
        assert len("\xff") == 1
        assert ord("\xff") == 255
        assert ord("\x0f") == 15

if __name__ == '__main__':
    tests = StringUnitTests()
    tests.run()

