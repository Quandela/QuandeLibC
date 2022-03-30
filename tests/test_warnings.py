import pytest
import numpy as np
import quandelibc as qc
import math
import warnings

@pytest.mark.filterwarnings("ignore:Casting complex values to real discards the imaginary part")
def test_warning():
    qc.permanent_fl(np.array([[1+1j]]))
