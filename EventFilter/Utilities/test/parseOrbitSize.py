import os, sys

f = open(sys.argv[1], "r")

lines = f.readlines()


def parse_title(l):
    fname = l[1]
    nevents  = l[3]

    return fname, nevents

totalCompressed = 0
totalUncompressed = 0
for i, l in enumerate(lines):
    l = l.strip().split()
    
    if(len(l)==0): continue

    if i==0:
        fname, nEvents = parse_title(l)
    elif i==1: continue
    elif i>1:
        totalCompressed += float(l[2])
        totalUncompressed += float(l[1])

print("Orbit Size (uncompressed/compressed): {}/{} [bytes/orbit]".format(
    int(totalUncompressed), int(totalCompressed)
    )
)

print("Throughput on disk: {:.2f} [MBytes/s]".format(
        totalCompressed*11_000 / 1024 / 1024
    )
)

f.close()
