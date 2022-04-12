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

import quandelibc as qc


def test_basic_0():
    fs = qc.FockState([0, 1])
    fs_mask = qc.FSMask(2, 1)
    assert fs_mask.match(fs)


def test_basic_1():
    fs_mask = qc.FSMask(2, 2, ["01"])
    assert fs_mask.match(qc.FockState([0, 1]))
    assert fs_mask.match(qc.FockState([0, 0]))
    assert not fs_mask.match(qc.FockState([0, 0]), allow_missing=False)
    assert not fs_mask.match(qc.FockState([1, 1]))
    assert not fs_mask.match(qc.FockState([1, 0]))
    assert not fs_mask.match(qc.FockState([0, 2]))
    assert not fs_mask.match(qc.FockState([2, 0]))


def test_basic_2():
    fs_mask = qc.FSMask(2, 2, [" 1"])
    assert fs_mask.match(qc.FockState([0, 1]))
    assert fs_mask.match(qc.FockState([1, 1]))
    assert fs_mask.match(qc.FockState([1, 0]))
    assert not fs_mask.match(qc.FockState([1, 0]), allow_missing=False)
    assert not fs_mask.match(qc.FockState([2, 0]))


def test_genfsa_1():
    # generate fsa array for 6 modes, and 4 photons
    fs_mask = qc.FSMask(6, 4, ["1    1"])
    fsa_full = qc.FSArray(6, 4)
    assert fsa_full.size() == 504
    fsa_restricted = qc.FSArray(6, 4, fs_mask)
    assert fsa_restricted.size() == 40


def test_genfsa_2():
    # generate fsa array for 12 modes, and 6 photons
    fs_mask = qc.FSMask(12, 6, ["111         "])
    fsa_full = qc.FSArray(12, 6)
    assert fsa_full.size() == 74256
    fsa_restricted = qc.FSArray(12, 6, fs_mask)
    assert fsa_restricted.size() == 990
