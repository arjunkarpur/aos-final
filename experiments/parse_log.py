import os
import json
from statistics import mean

FP = "logs/aida.json"
VERBOSE = False

ids = {
    1: "small_files_2p0",
    2: "small_files_2p1",
    3: "small_files_2p2",
    4: "small_files_2p3",
    5: "small_files_2p4",
    6: "small_files_2p5",
    7: "small_files_2p6",
    8: "small_files_2p7",
    9: "small_files_2p8",
    10: "small_files_2p9",
    11: "small_files_2p10",
    12: "small_files_2p11",
    13: "small_files_2p12",
    14: "small_files_2p13",
    15: "small_files_2p14",
    16: "small_files_2p15",
    17: "short_balanced_tree",
    18: "tall_balanced_tree",
    19: "tall_balanced_tree_more_files",
    20: "tall_balanced_tree_even_more_files",
    21: "tall_imbalanced_tree_more_files",
}

f = open(FP, 'r')
data = json.load(f)
f.close()

batch_votes = {}
for case in data:
    ours_data = case['ours']
    base_data = case['base']
    case_name = ids[case['case_id']]
    print("Test case: %s" % case_name)

    base_avg = mean(base_data)
    print("\tBase Avg:\t %f s" % base_avg)

    keys = [int(k) for k in ours_data.keys()]
    keys = sorted(keys)
    keys_str = [str(k) for k in keys]
    best_batch = -1
    best_time = float('inf')
    for bsize_s in keys_str:
        bsize = int(bsize_s)
        b_avg = mean(ours_data[bsize_s])
        tack = " "
        if b_avg < base_avg:
            tack = ">"
        if VERBOSE:
            print("\tOurs %i Avg:\t%s%f s" % (bsize, tack, b_avg))
        if b_avg < best_time:
            best_time = b_avg
            best_batch = bsize
    if not VERBOSE:
        print("\tOurs %i Avg:\t%s%f s" % (best_batch, " ", best_time))

    difference = (best_time - base_avg) / base_avg
    print("\tDiff: %f%%" % (difference*100))
    print("")

    if best_batch not in batch_votes:
        batch_votes[best_batch] = 0
    batch_votes[best_batch] += 1

print("Batch winners: (key=batch_size: value=count)")
print batch_votes
