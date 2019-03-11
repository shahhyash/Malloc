#!/usr/bin/python3
from matplotlib import pyplot as plt

def graph(test, data, color):
    mean = float(sum(data)) / float(len(data))
    fig = plt.figure(figsize=(8, 6), dpi=80)
    fig.text(.6,.8, "Mean: " + str(mean))
    X = [j for j in range(100)]
    plt.plot(X, data, c=color)
    plt.title("Graph " + test)
    plt.ylabel("Time (in seconds)")
    plt.xlabel("Iteration Number")
    #plt.show()
    plt.savefig("graph_" + test +".png")
    return

def main():
        fp = open('data.txt', 'r')
        A = []
        B = []
        C = []
        D = []
        E = []
        F = []
        i = 0
        for line in fp:
            line = line.split()[0]
            if i < 100:
                A.append(float(line))
            elif i >= 100 and i < 200:
                B.append(float(line))
            elif i >= 200 and i < 300:
                C.append(float(line))
            elif i >= 300 and i < 400:
                D.append(float(line))
            elif i >= 400 and i < 500:
                E.append(float(line))
            elif i >= 500 and i < 600:
                F.append(float(line))
            i += 1
        fp.close()

        for test, data, color in zip(['A','B','C','D','E','F'], [A,B,C,D,E,F], ['r','g','b','orange','black','purple']):
            graph(test, data, color)
        return

if __name__ == '__main__':
        main()
