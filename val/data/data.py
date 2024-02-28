import numpy as np
import matplotlib.pyplot as plt

def read_file(fn):
    data = {}
    data[0] = {}
    data[-1] = {}
    size = 0
    test_mte = 0
    with open(fn) as f:
        for line in f:
            if("testmte" in line):
                test_mte = int(line.strip().split()[1])
            elif("buffer_size" in line):
                size = int(line.strip().split()[1])
                data[test_mte][size] = {}
            elif("Setup" in line):
                curr = float(line.strip().split()[-2])
                data[test_mte][size].setdefault("Setup", []).append(int(curr))
            elif("Apply MTE" in line):
                curr = float(line.strip().split()[-2])
                data[test_mte][size].setdefault("ApplyMTE", []).append(int(curr))
            elif("Fill buffer" in line):
                curr = float(line.strip().split()[-2])
                data[test_mte][size].setdefault("FillBufferPC", []).append(int(curr))
            elif("Access" in line):
                curr = float(line.strip().split()[-2])
                data[test_mte][size].setdefault("Access", []).append(int(curr))
            elif("Memset" in line):
                curr = float(line.strip().split()[-2])
                data[test_mte][size].setdefault("Memset", []).append(int(curr))
            elif("Tear down" in line):
                curr = float(line.strip().split()[-2])
                data[test_mte][size].setdefault("TearDown", []).append(int(curr))
    f.close()
    return data

def plot(data, name, exp):

    mte = []
    no_mte = []
    size = []
    for s in data[0]:
        size.append(s)
        mte.append(np.mean(data[0][s][name][10:]))
    
    plot_no_mte = 0
    for s in data[-1]:
        if(name in data[-1][s]):
            plot_no_mte = 1
            no_mte.append(np.mean(data[-1][s][name][10:]))
        

    plt.figure(figsize=(3, 2))  # 6.4, 4.8
    plt.xscale('log')
    plt.xlabel('size(Byte)')
    plt.ylabel('Latency')
    plt.scatter(size, mte, label="mte")
    if(plot_no_mte):
        plt.scatter(size, no_mte, label="no_mte")
    plt.legend(fontsize=7)
    plt.tight_layout(pad=0.1)
    plt.savefig("./%s_%d.pdf" % (name, exp), dpi=300)

data = read_file("./exp3.txt")
plot(data, "TearDown", 3)

