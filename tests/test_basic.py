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
import numpy as np
import quandelibc as qc
import math


def test_main():
    assert qc.__version__
    assert hasattr(qc, "permanent_in")
    assert hasattr(qc, "permanent_fl")
    assert hasattr(qc, "permanent_cx")


def test_errorhandling_dim():
    with pytest.raises(RuntimeError):
        qc.permanent_fl(np.array([1]))


def test_basic_re():
    assert qc.permanent_fl(np.array([[1]])) == 1
    assert qc.permanent_fl(np.array([[1,1],[1,1]])) == 2
    assert qc.permanent_fl(np.array([[1,0,1],[1,0,1],[1,0,1]])) == 0


def test_sub_permanents():
    assert np.allclose(qc.sub_permanents_fl(np.array([[1], [2]])), np.array([2, 1]))
    assert np.allclose(qc.sub_permanents_fl(np.array([[1,2],[3,4],[5,6]])), np.array([38., 16., 10.]))


def test_factorial():
    for n in range(3,14):
        assert qc.permanent_fl(np.ones((n,n), dtype=float)) == math.factorial(n), "invalid calculation for dim %d" % n
    # reaching double max precision for !15
    for n in range(15,20):
        assert qc.permanent_in(np.ones((n,n), dtype=float)) == math.factorial(n), "invalid calculation for dim %d" % n
    # reaching long long (64-bits) precision for !21
