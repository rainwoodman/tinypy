from tinypy.runtime.testing import UnitTest

class MyClass:
    def __init__(self):
        self.__dict__['internal'] = {}

    def __set__(self, k, v):
        self.internal[k] = v

    def f(self):
        return 3

    def __get__(self, k):
        return k

class MyBypass:
    def __set__(self, k, v):
        return True

class MyDerived(MyClass):
    def __init__(self):
        self.__dict__['derived'] = True
        MyClass.__init__(self)

class MyTest(UnitTest):

    def test_rawdict(self):
        # __dict__ is the raw dict
        obj = MyClass()
        r1 = obj.__dict__
        r2 = getraw(obj)
        r1['foo'] = 1234
        assert r2['foo'] == 1234
        assert obj['foo'] == 1234

    def test_get_set(self):
        # get / set override logic

        # get occurs after default lookup
        # set occurs before default lookup

        obj = MyClass()
        obj.set_attr = '1234'
        obj.internal = 'foo'
        assert obj.f() == 3
        assert obj.unset_attr == "unset_attr"
        assert obj.internal['set_attr'] == '1234'
        assert obj.internal['internal'] == 'foo'
        assert obj.set_attr == "set_attr"

    def test_bypass(self):
        obj = MyBypass()
        obj.bypass = 300
        assert obj.__dict__['bypass'] == 300
        assert obj.bypass == 300

    def test_derived(self):
        obj = MyDerived()
        obj.set_attr = '1234'
        obj.internal = 'foo'

        assert obj.derived == True
        assert obj.unset_attr == "unset_attr"
        assert obj.internal['set_attr'] == '1234'
        assert obj.internal['internal'] == 'foo'
        assert obj.set_attr == "set_attr"

t = MyTest()

t.run()
