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
from os import path


def test_basic_fsa():
    fsa = qc.FSArray(20, 10)
    assert fsa.count() == 20030010
    assert fsa.m == 20
    assert fsa.n == 10


def test_fsa_n0():
    # for 0 photons we have a single state
    fsa = qc.FSArray(16, 0)
    assert fsa.size() == 0
    assert fsa.count() == 1
    assert fsa.m == 16
    assert fsa.n == 0
    assert fsa[0] == qc.FockState([0] * 16)
    assert fsa.find(qc.FockState([0] * 16)) == 0


def test_fsa_n0_iter():
    assert len(list(qc.FSArray(2, 0))) == 1


def test_save_fsa(tmp_path):
    fsa = qc.FSArray(3, 2)
    fsa.save(str(tmp_path / "fsa-32"))
    assert path.exists(tmp_path / "fsa-32")
    with open(tmp_path / "fsa-32") as f:
        s = f.read()
    assert s[:3] == "FSA"
    assert s[4:5] == "6"
    assert s[5] == '\x00'
    assert s[8:] == "AAABACBBBCCC"


def test_save_fsa_dir(tmp_path):
    fsa = qc.FSArray(3, 2)
    fsa.save(str(tmp_path))
    assert path.exists(tmp_path / "layer-m3-n2.fsa")
    with open(tmp_path / "layer-m3-n2.fsa") as f:
        s = f.read()
    assert s[:3] == "FSA"
    assert ord(s[6]) == 3
    assert ord(s[7]) == 2
    assert s[8:] == "AAABACBBBCCC"


def test_read_save(tmp_path):
    fsa1 = qc.FSArray(5, 4)
    fsa1.save(str(tmp_path / "fsa-54"))
    fsa2 = qc.FSArray(str(tmp_path / "fsa-54"), 5, 4)
    assert fsa2.count() == fsa1.count()
    assert fsa2[17] == fsa1[17]
    got_exception = False
    try:
        qc.FSArray(str(tmp_path / "fsa-54"), 6, 3)
    except ValueError:
        got_exception = True
    assert got_exception, "did not get exception"


def test_read_unknown(tmp_path):
    with pytest.raises(RuntimeError):
        qc.FSArray(str(tmp_path / "unknown-fsa"), 5, 4)


def test_fsm_basic():
    fsa_parent = qc.FSArray(3, 1)
    fsa = qc.FSArray(3, 2)
    fsm = qc.FSMap(fsa, fsa_parent)
    assert fsm.size() == 9
    for i in range(3):
        for j in range(3):
            print(i, j, fsm.get(i, j), fsa[fsm.get(i, j)])
    assert fsm.get(2, 2) == 5


def test_lazy_fsm():
    # the following cannot be built in memory - but if we pass, it means that we did not try
    fsa_parent = qc.FSArray(32, 15)
    fsa = qc.FSArray(32, 16)
    fsm = qc.FSMap(fsa, fsa_parent)
    assert fsm.size() == 98253842024448


def test_fsm_get_too_large():
    with pytest.raises(IndexError):
        fsa_parent = qc.FSArray(6, 2)
        fsa = qc.FSArray(6, 3)
        fsm = qc.FSMap(fsa, fsa_parent)
        fsm.get(45, 6)


def test_fsa_find_wrong_m():
    with pytest.raises(ValueError):
        fsa = qc.FSArray(6, 3)
        fsa.find(qc.FockState([1, 0, 0, 0, 0]))


def test_fsa_find_wrong_n():
    fsa = qc.FSArray(6, 3)
    assert fsa.find(qc.FockState([1, 0, 0, 0, 0, 0])) == qc.npos


def test_fsa_find_right():
    fsa = qc.FSArray(14, 7)
    idx = fsa.find(qc.FockState([1, 0, 1, 0, 2, 1, 1, 0, 1, 0, 0, 0, 0, 0]))
    assert fsa[idx] == qc.FockState([1, 0, 1, 0, 2, 1, 1, 0, 1, 0, 0, 0, 0, 0])
    # lexicographic order on the position
    assert fsa[idx + 1] == qc.FockState([1, 0, 1, 0, 2, 1, 1, 0, 0, 1, 0, 0, 0, 0])


def test_fsa_iter_0():
    fsa = qc.FSArray(3, 2)
    fsa_states = [
        "|2,0,0>",
        "|1,1,0>",
        "|1,0,1>",
        "|0,2,0>",
        "|0,1,1>",
        "|0,0,2>"
    ]
    assert fsa_states == [str(fs) for fs in fsa]


def test_fsa_find_boundary():
    fsa = qc.FSArray(2, 1)
    assert fsa.find(qc.FockState([1, 0])) == 0
    assert fsa.find(qc.FockState([0, 1])) == 1


def test_fsa_iter_1():
    fsa = qc.FSArray(3, 1)
    fsa_states = [
        "|1,0,0>",
        "|0,1,0>",
        "|0,0,1>"
    ]
    assert fsa_states == [str(fs) for fs in fsa]
