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

kb = 1024.
Mb = 1024.*1024
Gb = 1024.*1024*1024
Tb = 1024.*1024*1024*1024.

def format_unit(v):
    if v>100*Gb:
        return "%.2fTb" % (v/Tb)
    if v>100*Mb:
        return "%.2fGb" % (v/Gb)
    if v>100*kb:
        return "%.2fMb" % (v/Mb)
    return "%.2fKb" % (v/kb)

for m in range(50,51):
    values = []
    current_value = 0
    for n in range(1, m):
        fsa = qc.FSArray(m, n)
        fsm = qc.FSMap(fsa)
        current_value += fsm.size()
        values.append(current_value+fsa.size())
    print(str(m)+" "+" ".join([format_unit(v) for v in values]))
