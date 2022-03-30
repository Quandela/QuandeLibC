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
