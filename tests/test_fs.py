# -*- coding: utf-8 -*-
# MIT License
#
# Copyright (c) 2022 Quandela
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import pytest
import quandelibc as qc
import copy


def test_basic_0():
    fs = qc.FockState(2)
    assert str(fs) == "|0,0>"


def test_basic_ndef():
    fs = qc.FockState(2)
    fs += 1
    assert str(fs) == "|,>"


def test_parsing_0():
    fs = qc.FockState("|0,1,0>")
    assert str(fs) == "|0,1,0>"


def test_parsing_1():
    fs = qc.FockState("[0, 1, 0]")
    assert str(fs) == "|0,1,0>"


def test_parsing_2():
    fs = qc.FockState("|0, 1, 0〉")
    assert str(fs) == "|0,1,0>"


def test_init_from_fockstate():
    fs = qc.FockState("|0,1>")
    fs2 = qc.FockState(fs)
    assert str(fs2) == "|0,1>"


def test_parsing_3():
    with pytest.raises(ValueError):
        qc.FockState("|0, 1, 0]")


def test_parsing_4():
    with pytest.raises(ValueError):
        qc.FockState("{0, 1, 0}")


def test_add_error():
    with pytest.raises(ValueError):
        fs1 = qc.FockState([0, 1, 0])
        fs2 = qc.FockState([0, 0])
        fs1 + fs2


def test_add_ok_1():
    fs1 = qc.FockState([0, 1, 0])
    fs2 = qc.FockState([1, 0, 0])
    fs3 = fs1 + fs2
    assert str(fs3) == "|1,1,0>"


def test_add_ok_2():
    fs1 = qc.FockState([0, 1, 0])
    fs2 = qc.FockState([1, 0, 0])
    fs1 += fs2
    fs2 += fs1
    assert str(fs1) == "|1,1,0>"
    assert str(fs2) == "|2,1,0>"


def test_eq_1():
    fs1 = qc.FockState([0, 1, 0])
    fs2 = qc.FockState(3)
    fs3 = fs1 + fs2

    assert fs3 == fs1
    assert not (fs3 != fs1)
    fs3 += fs1
    assert fs3 != fs1
    assert not (fs3 == fs1)


def test_eq_2():
    fs1 = qc.FockState([1, 0, 1, 0])
    assert fs1 != [1, 0, 1, 0]
    assert fs1 is not None
    assert 1010 != fs1


def test_hash():
    fs1 = qc.FockState([0, 1, 0])
    fs2 = qc.FockState([1, 0, 2])
    m = dict()
    m[fs1] = fs1.n
    m[fs2] = fs2.n
    assert m[fs1] == 1
    assert m[fs2] == 3


def test_cast():
    fs1 = qc.FockState([0, 1, 0])
    print(list(fs1))


def test_getitem():
    fs1 = qc.FockState([1, 2, 3, 0])
    assert fs1[0] == 1
    assert fs1[1] == 2
    assert fs1[2] == 3
    assert fs1[3] == 0
    # is not valid but is the default answer
    try:
        fs1[4] == 0
    except IndexError:
        pass
    else:
        assert False, "should have generated an exception"


def test_getslice():
    fs = qc.FockState([0, 1, 0, 2, 1, 1])
    assert fs[0:6] == fs
    assert fs[-3:-1] == qc.FockState([2, 1])
    assert fs[1:3] == qc.FockState([1, 0])
    assert fs[2:2] == qc.FockState()
    assert fs[1:6:2] == qc.FockState([1, 2, 1])
    assert fs[1:6:3] == qc.FockState([1, 1])


def test_setslice():
    fs = qc.FockState([0, 1, 0, 2, 1, 1])
    assert fs.set_slice(slice(2, 4), qc.FockState([3, 4])) == qc.FockState([0, 1, 3, 4, 1, 1])

def test_properties_1():
    fs1 = qc.FockState([1, 2, 3, 0])
    assert fs1.n == 6
    assert fs1.m == 4


def test_properties_2():
    fs1 = qc.FockState(5)
    assert fs1.n == 0
    assert fs1.m == 5


def test_properties_3():
    fs1 = qc.FockState([1, 2, 3, 0])
    fs2 = qc.FockState([0, 1, 0, 0])
    assert fs1.n == 6
    assert (fs1 + fs2).n == 7
    assert fs1.m == 4


def test_0_photons():
    fs = qc.FockState("|0,0>")
    assert str(fs) == "|0,0>"


def test_prodnfact1():
    fs1 = qc.FockState(5)
    assert fs1.prodnfact() == 1


def test_prodnfact2():
    fs1 = qc.FockState([1, 2, 3])
    assert fs1.prodnfact() == 12


def test_copy_0():
    fs1 = qc.FockState([1, 2, 3, 0])
    fs2 = fs1
    fs1 += fs1
    assert fs1 == qc.FockState([2, 4, 6, 0])
    assert fs2 == fs1


def test_copy_0bis():
    fs1 = qc.FockState([1, 2, 3, 0])
    fs2 = copy.copy(fs1)
    fs1 += fs1
    assert fs1 == qc.FockState([2, 4, 6, 0])
    assert fs2 == qc.FockState([1, 2, 3, 0])


def test_plus_0():
    fs1 = qc.FockState([1, 2, 3])
    following_fs = [
        "|1,1,4>",
        "|1,0,5>",
        "|0,6,0>",
        "|0,5,1>",
        "|0,4,2>",
        "|0,3,3>",
        "|0,2,4>",
        "|0,1,5>",
        "|0,0,6>"
    ]
    for s in following_fs:
        fs1 += 1
        assert str(fs1) == s


def test_plus_1():
    fs1 = qc.FockState([1, 2, 3])
    following_fs = [
        "|1,1,4>",
        "|1,0,5>",
        "|0,6,0>",
        "|0,5,1>",
        "|0,4,2>",
        "|0,3,3>",
        "|0,2,4>",
        "|0,1,5>",
        "|0,0,6>"
    ]
    for c, s in enumerate(following_fs):
        assert str(fs1 + (1 + c)) == s

def test_annotation():
    fs = qc.FockState("|2{P:H},0>")
    assert str(fs) == "|2{P:H},0>"
    assert fs.get_mode_annotations(0) == [qc.Annotation("P:H"), qc.Annotation("P:H")]
    fs.clear_annotations()
    assert str(fs) == "|2,0>"
    assert fs.get_mode_annotations(1) == []
