import os
import time
import shutil
import datetime
from collections import namedtuple

from test_case_gen import get_gen_func

# Globals
RUN_NAME = "test1"
EXP_SKIP_LIST = []
N_EXPERIMENTS = 5
CLEAN_SRC = True
VERBOSE = True

PROJ_DIR = os.path.join("..")
FS_DIR = os.path.join(PROJ_DIR, "experiments", "fs")
LOG_DIR = os.path.join(PROJ_DIR, "experiments", "logs")

TestCase = namedtuple("TestCase", "case_id gen_func")

##############################################
# Helpers

def log_print(s):
    print("[%s]\t %s" % (datetime.datetime.now(), s))

def create_test_cases():
    # Populate with test case objects
    test_cases = []
    for i in range(1, 2):
        if i in EXP_SKIP_LIST:
            continue
        test_cases.append(TestCase(i, get_gen_func(i)))
    return test_cases

def time_command(cmd):
    start = time.time()
    os.system(cmd)
    end = time.time()
    return end-start

def experiment_main(test_case):
    log_print("[TEST CASE %i]" % test_case.case_id)

    # Create file system according to test case
    log_print("\tCreating src directory tree")
    src_dir = os.path.join(FS_DIR, "%s_src_%i" % (RUN_NAME, test_case.case_id))
    dest_dir = os.path.join(FS_DIR, "%s_dest_%i" % (RUN_NAME, test_case.case_id))
    os.makedirs(src_dir)
    test_case.gen_func(src_dir)

    # Run tests for base cp -r
    #TODO: baseline should be the same version of coreutils, but without any modifications. fix this
    log_print("\tRunning test for baseline cp -r")
    base_times = []
    for _ in range(N_EXPERIMENTS):
        t = time_command("cp -r %s %s" % (src_dir, dest_dir))
        base_times.append(t)
        shutil.rmtree(dest_dir)
        if VERBOSE:
            log_print("\t      %i - %f" % (_+1, t))
    log_print("\t    Avg - %f" % (sum(base_times)/float(N_EXPERIMENTS)))

    # TODO: do we need to interleave tests? To account for differences in server state between the two sets of experiments

    # Run tests for our cp -r
    our_times = []
    log_print("\tRunning tests for our cp -r")
    for _ in range(N_EXPERIMENTS):
        t = time_command("%s -r %s %s" % (os.path.join(PROJ_DIR, "coreutils-8.31", "src", "cp"), src_dir, dest_dir))
        our_times.append(t)
        shutil.rmtree(dest_dir)
        if VERBOSE:
            log_print("\t      %i - %f" % (_, t))
    log_print("\t    Avg - %f" % (sum(our_times)/float(N_EXPERIMENTS)))

    # Clean up generated directory tree
    log_print("\tCleaning up")
    if CLEAN_SRC:
        shutil.rmtree(src_dir)
    return { "case_id": test_case.case_id, "base": base_times, "ours": our_times }

def log_results(results):
    if not os.path.isdir(LOG_DIR):
        os.makedirs(LOG_DIR)
    out_fp = os.path.join(LOG_DIR, "%s.txt" % RUN_NAME)
    f = open(out_fp, 'w')
    for res in results:
        f.write("%s\n" % res)
    f.close()

##############################################
# Main

def main():
    # Create all test cases
    test_cases = create_test_cases();

    # Run experiment loop on each test case
    results = []
    for tcase in test_cases:
        tc_results = experiment_main(tcase)
        results.append(tc_results)

    # Report results
    log_results(results)

if __name__ == "__main__":
    main()
