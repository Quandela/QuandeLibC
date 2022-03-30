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
